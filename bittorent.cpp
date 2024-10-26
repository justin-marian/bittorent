#include "clients/clients.h"
#include "server/server.h"

#include <fstream>
#include <thread>

using namespace std;

int main (int argc, char **argv) {
    // Initialize MPI with multithreading support
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    // Check if MPI supports multithreading
    if (provided < MPI_THREAD_MULTIPLE) {
        fprintf(stderr, "MPI nu are suport pentru multi-threading\n");
        exit(EXIT_FAILURE);
    }

    int numtasks, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == TRACKER_RANK) {
        tracker(numtasks, rank);
    } else {
        peer(numtasks, rank);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
