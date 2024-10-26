#pragma once

#ifndef PEER_CLIENTS_H
#define PEER_CLIENTS_H 1

#include "../include/upload.h"
#include "../include/download.h"

#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief Represents a peer client.
 *
 * @param numtasks The total number of MPI tasks.
 * @param rank The rank of the current client.
 */
void peer(int numtasks, int rank);

/**
 * @brief Reads client files from a text document and stores them in the appropriate data structures.
 *
 * @param files Reference to an unordered_map to store file hashes indexed by filename.
 * @param filesNo Reference to an integer to store the number of files.
 * @param fileNames Reference to a vector to store the filenames.
 * @param rank The rank of the current client.
 */
void read_client_files(
    std::unordered_map<std::string, hashes>& files,
    std::vector<std::string>& fileNames, int& filesNo, int rank);

/**
 * @brief Sends file information (filename and hashes) to the trackedfile.
 *
 * @param files Reference to an unordered_map containing file hashes indexed by filename.
 * @param rank The rank of the current client.
 */
void send_file(
    const std::unordered_map<std::string, hashes>& files, int rank);

#endif // PEER_CLIENTS_H
