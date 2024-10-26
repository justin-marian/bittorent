#include "clients.h"

#include <mpi.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>

using namespace std;

static char recvMsg;

/**
 * @brief Reads client files from a text document and stores them in the appropriate data structures.
 *
 * @param files Reference to an unordered_map to store file hashes indexed by filename.
 * @param filesNo Reference to an integer to store the number of files.
 * @param fileNames Reference to a vector to store the filenames.
 * @param rank The rank of the current client.
 */
void read_client_files(unordered_map<string, hashes>& files, vector<string>& fileNames, int& filesNo, int rank) {
    ifstream recvFile("in" + to_string(rank) + ".txt");
    string fileIn, fileName, lineNo;

    // Read the number of files
    if (getline(recvFile, fileIn)) {
        filesNo = stoi(fileIn);

        // Read file information line by line
        for (int fIdx = 0; fIdx < filesNo; ++fIdx) {
            getline(recvFile, fileIn);

            // Parse filename and number of hashes
            istringstream fileStream(fileIn);
            getline(fileStream, fileName, ' ');
            getline(fileStream, lineNo, ' ');

            int hashesNo = stoi(lineNo);
            vector<string> hashes(hashesNo);

            // Read hashes line by line
            for (int hash = 0; hash < hashesNo; ++hash) {
                getline(recvFile, fileIn);
                hashes[hash] = fileIn;
            }

            // Store filename and hashes in the unordered_map
            files[fileName] = {hashesNo, hashes};
        }
    }

    // Read the number of file names
    if (getline(recvFile, fileIn)) {
        filesNo = stoi(fileIn);
        fileNames.resize(filesNo);

        // Read file names line by line
        for (int fIdx = 0; fIdx < filesNo; ++fIdx) {    
            getline(recvFile, fileIn);
            fileNames[fIdx] = fileIn;
        }
    }

    recvFile.close();
}

/**
 * @brief Sends file information (filename and hashes) to the trackedfile.
 *
 * @param files Reference to an unordered_map containing file hashes indexed by filename.
 * @param rank The rank of the current client.
 */
void send_file(const unordered_map<string, hashes>& files, int rank) {
    int filesNo = files.size();
    MPI_Send(&filesNo, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

    // Iterate over each file and send its information to the trackedfile
    for (const auto& [fileName, data] : files) {
        MPI_Send(fileName.c_str(), MAX_FILENAME, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
        MPI_Send(&data.hashesNo, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

        // Send each hash line by line
        for (const auto& line : data.hashesCurr) {
            MPI_Send(line.c_str(), HASH_SIZE, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
        }
    }
}

/**
 * @brief Peer client, clients have numtasks different from 0.
 *
 * @param numtasks The total number of MPI tasks.
 * @param rank The rank of the current client.
 */
void peer(int numtasks, int rank) {
    unordered_map<string, hashes> files;
    vector<string> fileNames;
    int filesNo = 0;

    // Read client files and prepare for communication
    read_client_files(files, fileNames, filesNo, rank);
    // Send file information to the trackedfile and wait for acknowledgement
    send_file(files, rank);

    MPI_Recv(&recvMsg, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (recvMsg != ACK) {
        cout << "Trackedfile did not receive the data, client: " << rank << "\n";
    }

    // Initialize downloading and uploading threads
    thread download(download_thread, rank, filesNo, fileNames.data());
    thread upload(upload_thread, files, rank);
    download.join();
    upload.join();
}
