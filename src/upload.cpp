#include "../include/upload.h"

#include <mpi.h>
#include <iostream>

using namespace std;

static char recvMsg;
static MPI_Status status;

/**
 * @brief Handle shutdown signal from the coordinator
 * shutdown signal (FIN) is received, false otherwise
 */
void shutdown_upload(void) {
    // Receive the shutdown command from the coordinator
    MPI_Recv(&recvMsg, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

    if (recvMsg == FIN) {
        cout << "Shutdown, uploaded ended!\n";
        return; // Indicate shutdown upload communication
    }
}

/**
 * @brief Respond to segment request from clients
 */
void segment_request_response(void) {
    recvMsg = ACK;
    char *segment = new char [HASH_SIZE];

    // Receive the segment request from any source
    MPI_Recv(segment, HASH_SIZE, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);    
    // Send acknowledgment (ACK) to the source
    MPI_Send(&recvMsg, 1, MPI_CHAR, status.MPI_SOURCE, 1, MPI_COMM_WORLD);

    delete[] segment;
}

/**
 * @brief Thread function to handle upload tasks
 * 
 * @param files Map containing file information
 * @param rank Rank of the current MPI process
 */
void upload_thread(unordered_map<string, hashes> files, int rank) {
    bool stopUpload = false;
    int flag = 0;

    while (!stopUpload) {
        // Check for incoming messages without blocking
        MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        // Test if a message is received
        MPI_Get_count(&status, MPI_CHAR, &flag);

        if (flag) { // If there's an incoming message
            if (status.MPI_SOURCE == 0) {
                // Handle shutdown signal from source 0
                stopUpload = true;
                shutdown_upload();
            } else {
                // Process segment requests from other sources
                segment_request_response();
            }
        }
    }
}
