#include "../include/download.h"

#include <mpi.h>
#include <thread>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>

using namespace std;

static char recvMsg;

/**
 * @brief Sends file information to the coordinator.
 * 
 * @param fileNo The number of files to send information for.
 * @param files Array of file names.
 * @param rank The rank of the current MPI task.
 */
void send_file_swarm(int fileNo, string* files, int rank) {
    // Send the number of new files
    MPI_Send(&fileNo, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

    char fileName[MAX_FILENAME];

    // Send all the fileNames needed
    for (int fIdx = 0; fIdx < fileNo; ++fIdx) {
        memset(fileName, 0, sizeof(char) * MAX_FILENAME);
        strncpy(fileName, files[fIdx].c_str(), MAX_FILENAME);
        MPI_Send(&fileName, MAX_FILENAME, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
    }
}

/**
 * @brief Receives file swarm information from the coordinator.
 * 
 * @param segmentsNo Reference to store the number of segments.
 * @param rank The rank of the current MPI task.
 * @return The received file swarm information.
 */
trackedfile receive_file_swarm(int& segmentsNo, int rank) {
    trackedfile swarm;

    // Receive the number of members and the number of segments
    MPI_Recv(&segmentsNo, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&swarm.segmentsNo, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Resize the providers vector to accommodate the received number of members
    swarm.providers.resize(segmentsNo);

    // Receive the providers data directly into the vector
    MPI_Recv(swarm.providers.data(), segmentsNo * sizeof(client), MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Receive segment hashes and store them in the swarm
    for (int sidx = 0; sidx < swarm.segmentsNo; sidx++) {
        char *hash = (char *) malloc(sizeof(char) * HASH_SIZE);
        MPI_Recv(hash, HASH_SIZE, MPI_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        swarm.segments.push_back(hash);
    }

    return swarm;
}

/**
 * @brief Processes segments of a file.
 * 
 * @param files Array of file names.
 * @param rank The rank of the current MPI task.
 * @param segmentLast Reference to the last processed segment.
 * @param fIdx Index of the current file being processed.
 * @param swarm Reference to the file swarm data.
 * @param segmentsNo The total number of segments in the file.
 */
void process_file_segments(string* files, int rank, int& segmentLast, int fIdx, trackedfile& swarm, int segmentsNo) {
    char fileName[MAX_FILENAME];
    strcpy(fileName, files[fIdx].c_str());

    // Shuffle the providers vector to randomize the order of selection
    random_device rd;
    mt19937 g(rd());
    shuffle(swarm.providers.begin(), swarm.providers.end(), g);

    for (auto& client : swarm.providers) {
        if (client.interval.last > segmentLast) {
            // If the client has a last_hash greater than segmentLast, select it as seed
            int seeder = client.id;
            int sIdx;
            for (sIdx = segmentLast; sIdx < segmentLast + 10 && sIdx < swarm.segmentsNo; sIdx++) {
                MPI_Send(&fileName, MAX_FILENAME, MPI_CHAR, seeder, 0, MPI_COMM_WORLD);
                MPI_Recv(&recvMsg, 1, MPI_CHAR, seeder, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            segmentLast = sIdx;
            // Exit loop after processing segments
            break;
        }
    }
}

/**
 * @brief Finalizes file assembly and saves it.
 * 
 * @param files Array of file names.
 * @param rank The rank of the current MPI task.
 * @param segmentLast Reference to the last processed segment.
 * @param fIdx Index of the current file being processed.
 * @param swarm Reference to the file swarm data.
 */
void finalize_file_save(string* files, int rank, int& segmentLast, int fIdx, trackedfile& swarm) {
    char fileName[MAX_FILENAME];
    strcpy(fileName, files[fIdx].c_str());

    MPI_Send(&fileName, MAX_FILENAME, MPI_CHAR, 0, 2, MPI_COMM_WORLD);
    MPI_Send(&segmentLast, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);

    if (segmentLast == swarm.segmentsNo) {
        string clientFile = "client" + to_string(rank) + "_" + fileName;
        ofstream resultFile(clientFile);

        for (auto line : swarm.segments) {
            if (resultFile.is_open()) {
                line[HASH_SIZE] = '\0';
                resultFile << line << endl;
            }
        }
    }
}

/**
 * @brief Thread function to handle download tasks
 * 
 * @param rank The rank of the current MPI task.
 * @param fileNo The number of files to download.
 * @param fileNames Pointer to an array of file names.
 */
void download_thread(int rank, int fileNo, void* fileNames) {    
    string* files = (string*) fileNames;
    int segmentsNo = 0;
    int segmentLast = 0;
    int filesDownloaded =  0;

    // Send file information to the coordinator
    send_file_swarm(fileNo, files, rank);

    for (int fIdx = 0; fIdx < fileNo; ++fIdx) {
        // Receive file swarm information
        trackedfile swarm = receive_file_swarm(segmentsNo, rank);

        // While not all segments have been processed
        while (segmentLast < swarm.segmentsNo) {
            // Process a chunk of file segments
            process_file_segments(files, rank, segmentLast, fIdx, swarm, segmentsNo);
        }

        // Finalize file assembly and save it
        finalize_file_save(files, rank, segmentLast, fIdx, swarm);

        // Cleanup allocated memory for this file's data
        for (auto segment : swarm.segments) {
            free(segment);
        }

        // Reset last segment index for the next file
        segmentLast = 0;
    }

    // Notify the coordinator that this client has finished its downloads
    recvMsg = FIN;
    MPI_Send(&recvMsg, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    cout << "No. of files downloaded "
         << "(inclusive files that are not containing all the hashes): "
         << ++filesDownloaded << "\n";
}
