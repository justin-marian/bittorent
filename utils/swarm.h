#pragma once

#ifndef SWARM_H
#define SWARM_H

#include <string>
#include <vector>

#define TRACKER_RANK 0

enum peertype {
    SEED,
    PEER,
    LEECHER
};

struct hashrange {
    union {
        int start;  // Start index of the hash interval
        int first;  // Alias for start
    };
    union {
        int end;    // End index of the hash interval
        int last;   // Alias for end
    };
};

struct client {
    int id;             // Client ID
    peertype type;      // Type of client
    hashrange interval; // Interval index for segments owned
};

#endif // SWARM_H
