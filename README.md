# BitTorrent Protocol

## Simulation with MPI

This project aims to simulate the BitTorrent protocol for peer-to-peer (P2P) file sharing using `MPI` (Message Passing Interface). The simulation reflects the core mechanics of BitTorrent, where files are divided into segments and shared across multiple nodes. This setup allows for efficient, distributed data transfer without relying on a central server.

## Overview

The simulation has two primary components:

- **Tracker**: Acts as a central reference, managing file segment locations across the network and guiding peers to each other.
- **Clients (Peers)**: Download and share file segments with other clients, creating a cooperative data-sharing network.

## Logic and Mechanics

### 1. Tracker Responsibilities

The tracker (MPI `rank 0`) is responsible for coordinating data exchange without holding file data itself:

- **File Segment Indexing**: It tracks which peers hold each segment of a file.
- **Peer Connections**: When a client requests segments, the tracker provides a list of peers holding them.
- **Segment Updates**: It records updates from peers about new segments they acquire, ensuring current data for other clients.

This structure allows the tracker to guide clients to appropriate peers efficiently, minimizing redundant downloads.

### 2. Client Role and Threads

Each client has two active threads to manage its dual roles of data consumer and provider:

- **Download Thread**: Manages segment requests from the tracker, downloads segments from peers, and builds the file.
- **Upload Thread**: Responds to requests for segments, sharing data with other clients.

This setup promotes balanced network traffic, as each client actively participates in both downloading and uploading data.

### 3. Downloading Mechanism

The download logic is designed to optimize network efficiency and balance:

1. **Segment Requests**: Clients first check with the tracker for a list of peers holding required segments.
2. **Non-Sequential Segment Retrieval**: Clients download segments as available, reducing wait times and minimizing network bottlenecks.
3. **Load Balancing**: Clients dynamically vary peer sources for each segment, ensuring even data distribution and reducing overreliance on any single peer.

### 4. Updating the Tracker

Clients periodically update the tracker with their new segments, which helps maintain an accurate, real-time view of file segment distribution across the network.

- **Status Reporting**: Each client informs the tracker of segments it has downloaded.
- **Network-Wide Updates**: The tracker relays segment availability to clients needing the same files, helping new and existing peers locate data.

### 5. Completion and Network Persistence

Upon finishing a download, clients continue to support the network:

- **Seeding**: Completed clients serve segments to others, ensuring data remains available for newer peers.
- **Network Shutdown**: The tracker signals when all downloads are complete, allowing clients to exit gracefully.

## Fault Tolerance and Efficiency

The simulation includes strategies to enhance resilience and efficiency:

- **Redundant Connections**: Multiple peers are accessible for each segment, preventing interruptions if a peer disconnects.
- **Load Balancing**: Distributed download sources prevent any single client from becoming overloaded.
- **Non-Sequential Segmentation**: Clients can access available data without waiting on specific peers, maintaining high transfer speeds.

## Simulation Constraints and Simplifications

Certain real-world complexities are abstracted in this simulation:

- **Simulated Data Transfer**: File segments are represented as hash values for simplicity.
- **Controlled Environment**: Peer-to-peer requests are managed through MPI messages rather than actual network traffic, focusing on protocol behavior rather than data transmission.

### Benefits of MPI for Simulation

Using MPI enhances the simulation by providing:

- **Parallel Processing**: Multiple clients operate concurrently, mimicking a real-time P2P network.
- **Scalability**: MPI’s process-based design allows for scaling client interactions and file-sharing scenarios.
- **Structured Testing**: This controlled setup simplifies protocol testing and validation.
