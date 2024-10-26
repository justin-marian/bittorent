#pragma once

#ifndef UPLOAD_CLIENTS_H
#define UPLOAD_CLIENTS_H 1

#include "../utils/file_info.h"
#include "../utils/swarm.h"

#include <string>
#include <unordered_map>

/**
 * @brief Handle shutdown signal from the coordinator
 */
void shutdown_upload(void);

/**
 * @brief Respond to segment request from clients
 */
void segment_request_response(void);

/**
 * @brief Thread function to handle upload tasks
 * 
 * @param files Map containing file hashes segments
 * @param rank Rank of the current MPI process
 */
void upload_thread(
    std::unordered_map<std::string, hashes> files, int rank);

#endif // UPLOAD_CLIENTS_H
