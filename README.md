# BitTorrent Protocol

## Simulation with MPI

This project simulates the BitTorrent protocol for `P2P` file sharing using `MPI`.
The simulation captures core BitTorrent mechanics, distributing file segments across multiple nodes, allowing efficient and decentralized file sharing without relying on a central server.

## Overview

| Component   | Description                                                                                          |
|-------------|------------------------------------------------------------------------------------------------------|
| **Tracker** | Acts as a central node, managing file segment locations across the network and guiding peers.        |
| **Clients** | Download and upload file segments with other clients (**coperative data-sharing network**).          |

## Core Logic

### 1. Tracker Responsibilities

The tracker (`MPI rank 0`) coordinates data exchange but does not store file data itself:

- **File Segment Indexing**: Maintains a record of segment locations across peers.
- **Peer Connections**: Provides peers holding requested segments when clients inquire.
- **Segment Updates**: Tracks segment availability updates from clients, ensuring current data.

This setup allows the tracker to efficiently connect clients, minimizing redundant downloads.

### 2. Client Role and Threads

Each client operates with two active threads to balance its dual role as data consumer and provider:

| Thread              | Description                                                                                    |
|---------------------|------------------------------------------------------------------------------------------------|
| **Download Thread** | Manages segment requests from peers and builds the file.                                       |
| **Upload Thread**   | Responds to requests from other clients, sharing owned segments.                               |

This thread setup balances network traffic by allowing each client to both download and upload data.

### 3. Downloading Mechanism

The download process optimizes network efficiency:

- **Segment Requests**: Clients check with the tracker for peers holding needed segments.
- **Non-Sequential Retrieval**: Clients download available segments first, minimizing network wait times.
- **Load Balancing**: By varying peer sources, clients distribute load evenly across the network.

### 4. Updating the Tracker

Clients periodically report their segment updates to the tracker to maintain real-time segment availability across the network.

| Update Step         | Description                                                                                   |
|---------------------|-----------------------------------------------------------------------------------------------|
| **Status Reporting**| Clients report newly acquired segments to the tracker.                                        |
| **Network Updates** | The tracker informs other clients of updated segment availability.                            |

### 5. Completion and Network Persistence

After completing their downloads, clients remain active as seeds, helping maintain data availability for others:

| Completion Step     | Description                                                                                   |
|---------------------|-----------------------------------------------------------------------------------------------|
| **Seeding**         | Completed clients serve segments to new peers, sustaining availability.                       |
| **Shutdown**        | The tracker signals when all clients have completed downloads, allowing an orderly exit.      |

## Fault Tolerance and Efficiency

The simulation includes several strategies to ensure efficient and resilient data sharing:

| Feature                | Description                                                                              |
|------------------------|------------------------------------------------------------------------------------------|
| **Redundant Connections** | Multiple peers are accessible for each segment, preventing interruptions.             |
| **Load Balancing**        | Distributes requests, avoiding overload on any single client.                         |
| **Non-Sequential Retrieval** | Maintains high transfer speeds without waiting on specific peers.                   |

## Simulation Constraints and Simplifications

The simulation abstracts certain real-world complexities:

| Simplification         | Description                                                                              |
|------------------------|------------------------------------------------------------------------------------------|
| **Simulated Data Transfer** | Segments represented as hash values, focusing on protocol, not file transfer.       |
| **Controlled Environment**  | Peer requests managed via MPI messages, simplifying protocol behavior testing.      |

### Benefits of MPI for Simulation

- **Parallel Processing**: Multiple clients operate simultaneously, simulating real-time P2P interactions.
- **Scalability**: MPI’s process-based architecture supports scaling of client interactions.
- **Structured Testing**: Simplified testing of protocol operations and validations in a controlled environment.
