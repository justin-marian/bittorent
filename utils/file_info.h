#pragma once

#ifndef FILE_INFO_H
#define FILE_INFO_H 1

#include "swarm.h"

#include <vector>

#define HASH_SIZE 32
#define MAX_FILES 10

#define MAX_FILENAME 15
#define MAX_CHUNKS 100

#define FIN '0'
#define ACK '1'

struct hashes {
    int hashesNo;
    std::vector<std::string> hashesCurr;        
};

struct trackedfile {
    int segmentsNo;                    // Number of segments
    std::vector<char*> segments;       // All hashes needed
    std::vector<client> providers;     // Data hashes and client details
};

#endif // FILE_INFO_H
