#include "server.h"
#include <mpi.h>
#include <iostream>

using namespace std;

static MPI_Status status;
static char recvMsg;

/**
 * @brief Receive the number of files from a client.
 *
 * @param cIdx The index of the client.
 * @param fileNo Reference to store the number of files received.
 */
static void recv_file_no(int cIdx, int& fileNo) {
    if (MPI_Recv(&fileNo, 1, MPI_INT, cIdx, 1, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
        cerr << "[ERROR]: receiving file number from client " << cIdx << "\n";
    }
}

/**
 * @brief Receive the file name from a client.
 *
 * @param cIdx The index of the client.
 * @param fileName Pointer to store the received file name.
 */
static void recv_file_name(int cIdx, char* fileName) {
    if (MPI_Recv(fileName, MAX_FILENAME, MPI_CHAR, cIdx, 1, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
        cerr << "[ERROR]: receiving filename from client " << cIdx << "\n";
    }
}

/**
 * @brief Receives client information including segments.
 *
 * @param cIdx The index of the client.
 * @param swarm Reference to the trackedfile object to store client information.
 */
static void recv_segments_file(int cIdx, trackedfile& swarm) {
    if (MPI_Recv(&swarm.segmentsNo, 1, MPI_INT, cIdx, 1, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
        cerr << "[ERROR]: receiving segmentsNo from client " << cIdx << endl;
    }

    // Efficiently reserve space for known quantities
    swarm.providers.reserve(swarm.providers.size() + 1);
    swarm.segments.reserve(swarm.segmentsNo);

    client clientDetails = {cIdx, SEED, 0, swarm.segmentsNo};
    swarm.providers.push_back(clientDetails);

    for (int sIdx = 0; sIdx < swarm.segmentsNo; ++sIdx) {
        char *hash = new char[HASH_SIZE];
        if (MPI_Recv(hash, HASH_SIZE, MPI_CHAR, cIdx, 1, MPI_COMM_WORLD, &status) != MPI_SUCCESS) {
            cerr << "[ERROR]: receiving segment hash from client " << cIdx << "\n";
            continue;
        }
        swarm.segments.push_back(hash);
    }
}

/**
 * @brief Broadcasts shutdown signal to all clients.
 *
 * @param numtasks Total number of tasks including the tracker.
 */
void shutdown(int numtasks) {
    char close = FIN; // Broadcast confirmation to clients
    for (int cIdx = 1; cIdx < numtasks; ++cIdx) {
        MPI_Send(&close, 1, MPI_CHAR, cIdx, 0, MPI_COMM_WORLD);
    }
}

/**
 * @brief Broadcasts confirmation signal to all clients.
 *
 * @param numtasks Total number of tasks including the tracker.
 */
void confirmation(int numtasks) {
    char load = ACK; // Broadcast confirmation to clients
    for (int cIdx = 1; cIdx < numtasks; ++cIdx) {
        MPI_Send(&load, 1, MPI_CHAR, cIdx, 1, MPI_COMM_WORLD);
    }
}

/**
 * @brief Receives data from clients regarding the number of leechers and files.
 *
 * @param numtasks Total number of tasks including the tracker.
 * @param leechersNo Reference to store the number of leechers.
 * @param filesNo Reference to store the number of files.
 */
void recv_data_from(int numtasks, int& leechersNo, int& filesNo) {
    leechersNo = numtasks - 1;
    for (int cIdx = 1; cIdx < numtasks; ++cIdx) {
        MPI_Recv(&filesNo, 1, MPI_INT, cIdx, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (filesNo == 0) {
            leechersNo--;
        }
    }
}

/**
 * @brief Sends swarm data to a client.
 *
 * @param swarm Reference to the trackedfile object containing swarm data.
 * @param rank The index of the client receiving the swarm data.
 */
void send_data_to(const trackedfile& swarm, int rank) {
    int providers = swarm.providers.size();
    cout << "\n\n Send request to client: " << rank << "\n";

    MPI_Send(&providers, 1, MPI_INT, rank, 1, MPI_COMM_WORLD);
    MPI_Send(&swarm.segmentsNo, 1, MPI_INT, rank, 1, MPI_COMM_WORLD);
    MPI_Send(swarm.providers.data(), providers * sizeof(client), MPI_BYTE, rank, 1, MPI_COMM_WORLD);
    
    // Send hash segments
    for (int sIdx = 0; sIdx < swarm.segmentsNo; ++sIdx) {
        MPI_Send(swarm.segments[sIdx], HASH_SIZE, MPI_CHAR, rank, 1, MPI_COMM_WORLD);
    }
}

/**
 * @brief Receives data from clients and updates the database with file information.
 *
 * @param numtasks Total number of tasks including the tracker.
 * @param database Reference to the unordered map storing file information.
 * @param leechersFiles Reference to the unordered map storing the number of leechers for each file.
 */
void update_request(int numtasks,
    unordered_map<string, trackedfile>& database,
    unordered_map<string, int>& leechersFiles) {

    int fileNo = 0;
    char fileCName[MAX_FILENAME];

    for (int cIdx = 1; cIdx < numtasks; ++cIdx) {
        // Receive file number
        recv_file_no(cIdx, fileNo);

        for (int fIdx = 0; fIdx < fileNo; ++fIdx) {
            // Receive file name
            recv_file_name(cIdx, fileCName);
            string fileName(fileCName);
            // Receive client information
            trackedfile swarm;
            recv_segments_file(cIdx, swarm);

            // Update database
            database[fileName] = swarm;
            // Ensures a new file entry or an update to an existing one
            leechersFiles[fileName] = 0;
        }
    }
}

/**
 * @brief Receives the finished downloading signal from a client and updates the database accordingly.
 *
 * @param database Reference to the unordered map storing file information.
 * @param leechersFiles Reference to the unordered map storing the number of leechers for each file.
 */
void update_databe(
    unordered_map<string, trackedfile>& database,
    unordered_map<string, int>& leechersFiles) {

    char fileCName[MAX_FILENAME];
    int segmentLast;

    // Client finished downloading
    MPI_Recv(fileCName, MAX_FILENAME, MPI_CHAR, status.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&segmentLast, 1, MPI_INT, status.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    string fileName(fileCName);
    if (segmentLast == 10) {
        // Add new seed
        client seeder;
        seeder.id = status.MPI_SOURCE;
        seeder.interval.first = 10;
        seeder.interval.last = 0;
        seeder.type = PEER;
        database[fileName].providers.push_back(seeder);
    } else if (segmentLast < database[fileName].segmentsNo && segmentLast > 10) {
        // Update the last segment for the client
        for (auto& it : database[fileName].providers) {
            if (it.id == status.MPI_SOURCE) {
                it.interval.last = segmentLast;
            }
        }
    } else if (segmentLast == database[fileName].segmentsNo) {
        // Client becomes a seed
        leechersFiles[fileName]--;
        for (auto& it : database[fileName].providers) {
            if (it.id == status.MPI_SOURCE) {
                it.interval.last = segmentLast;
                it.type = SEED;
            }
        }
    }

    // Logging for debugging purposes
    cout << fileName
         << " client " << status.MPI_SOURCE << "\n"
         << "last hash segment " << segmentLast
         << " total " << database[fileName].segmentsNo << "\n";

    for (const auto& it : database[fileName].providers) {
        cout << "client id: " << it.id << "\n"
             << "last hash segment: " << it.interval.last << "\n"
             << "client type: " << it.type << "\n";
    }
    cout << "\n\n";
}

/**
 * @brief Main function for the tracker. 
 * Manages the data exchange between clients and handles completion signals.
 *
 * @param numtasks Total number of tasks including the tracker.
 * @param rank Rank of the current task.
 */
void tracker(int numtasks, int rank) {
    unordered_map<string, trackedfile> database;
    unordered_map<string, int> leechersFiles;
    
    int inSwarm = 0, filesNo = -1, leechersNo = numtasks - 1;
    char fileCName[MAX_FILENAME];

    // Initial data gathering and confirmation
    update_request(numtasks, database, leechersFiles);
    confirmation(numtasks);
    recv_data_from(numtasks, leechersNo, filesNo);

    // Loop until all leechers have finished downloading
    while (inSwarm < leechersNo) {
        memset(fileCName, 0, sizeof(char) * MAX_FILENAME);
        // Receive file name from any client
        MPI_Recv(fileCName, MAX_FILENAME, MPI_CHAR, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);

        string fileName(fileCName);
        // Get swarm information for the file
        trackedfile swarm = database[fileName];
        // Send swarm information to the client
        send_data_to(swarm, status.MPI_SOURCE);
        // Process finished downloading message from the client
        update_databe(database, leechersFiles);

        // Handle finish message
        cout << "Received request from: client" << status.MPI_SOURCE << "\n";
        // Probe for incoming message
        MPI_Probe(status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == 0) {
            // Receive finish confirmation from any source
            MPI_Recv(&recvMsg, 1, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            // Check if it's a finish message
            if (recvMsg == FIN) {
                inSwarm++;
            }
        }
        cout << inSwarm << "  ||  " << numtasks << "\n";
    }

    // Finalize all clients
    shutdown(numtasks);

    // Free allocated memory
    for (auto& file : database) {
        for (auto& segments : file.second.segments) {
            free(segments);
        }
    }
}
