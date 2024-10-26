#pragma once

#ifndef TRACKER_SERVER_H
#define TRACKER_SERVER_H 1

#include "../include/upload.h"
#include "../include/download.h"

#include <mpi.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <unordered_map>

/**
 * @brief Main function for the tracker.
 * Manages the data exchange between clients and handles completion signals.
 *
 * @param numtasks Total number of tasks including the tracker.
 * @param rank Rank of the current task.
 */
void tracker(int numtasks, int rank);

/**
 * @brief Sends shutdown signal to all clients.
 *
 * @param numtasks Total number of tasks including the tracker.
 */
void shutdown(int numtasks);

/**
 * @brief Sends confirmation signal to all clients.
 *
 * @param numtasks Total number of tasks including the tracker.
 */
void confirmation(int numtasks);

/**
 * @brief Receives data from clients and updates the database with file information.
 *
 * @param numtasks Total number of tasks including the tracker.
 * @param database Reference to the unordered map storing file information.
 * @param leechersFiles Reference to the unordered map storing the number of leechers for each file.
 */
void update_request(
    int numtasks,
    std::unordered_map<std::string, trackedfile>& database,
    std::unordered_map<std::string, int>& leechersFiles);

/**
 * @brief Receives the finished downloading signal from a client and updates the database accordingly.
 *
 * @param database Reference to the unordered map storing file information.
 * @param leechersFiles Reference to the unordered map storing the number of leechers for each file.
 */
void update_databe(
    std::unordered_map<std::string, trackedfile>& database,
    std::unordered_map<std::string, int>& leechersFiles);


/**
 * @brief Receives the number of leechers and files from each client.
 *
 * @param numtasks Total number of tasks including the tracker.
 * @param leechersNo Reference to the variable storing the total number of leechers.
 * @param filesNo Reference to the variable storing the total number of files.
 */
void recv_data_from(int numtasks, int& leechersNo, int& filesNo);

/**
 * @brief Sends data to a client.
 *
 * @param swarm Reference to the trackedfile object containing swarm information
 * (number of segments, segment hashes and providers).
 * @param rank Rank of the current task.
 */
void send_data_to(const trackedfile& swarm, int rank);

#endif // TRACKER_SERVER_H
