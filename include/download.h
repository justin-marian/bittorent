#pragma once

#ifndef DOWNLOAD_CLIENTS_H
#define DOWNLOAD_CLIENTS_H 1

#include "../utils/file_info.h"

#include <string>
#include <unistd.h>

/**
 * @brief Main function for the download thread.
 * 
 * @param rank The rank of the current MPI task.
 * @param fileNo The number of files to download.
 * @param fileNames Pointer to an array of file names.
 */
void download_thread(int rank, int fileNo, void* fileNames);

/**
 * @brief Receives file swarm information from the coordinator.
 * 
 * @param segmentsNo Reference to store the number of members.
 * @param rank The rank of the current MPI task.
 * @return The received file swarm information.
 */
trackedfile receive_file_swarm(int& segmentsNo, int rank);

/**
 * @brief Sends file swarm information to the coordinator.
 * 
 * @param fileNo The number of files to send information for.
 * @param files Array of file names.
 * @param rank The rank of the current MPI task.
 */
void send_file_swarm(int fileNo, std::string* files, int rank);

/**
 * @brief Processes segments of a file.
 * 
 * @param files Array of file names.
 * @param rank The rank of the current MPI task.
 * @param segmentLast Reference to the last processed segment.
 * @param fIdx Index of the current file being processed.
 * @param swarm Reference to the file swarm data.
 * @param segmentsNo The total number of members.
 */
void process_file_segments(
    std::string* files, int rank,
    int& segmentLast, int fIdx,
    trackedfile& swarm, int segmentsNo);

/**
 * @brief Finalizes file assembly and saves it.
 * 
 * @param files Array of file names.
 * @param rank The rank of the current MPI task.
 * @param segmentLast Reference to the last processed segment.
 * @param fIdx Index of the current file being processed.
 * @param swarm Reference to the file swarm data.
 */
void finalize_file_save(
    std::string* files, int rank,
    int& segmentLast, int fIdx,
    trackedfile& swarm);

#endif // DOWNLOAD_CLIENTS_H
