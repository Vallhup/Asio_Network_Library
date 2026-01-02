# Asio_Network_Module

## 1. Overview

This Project aims to develop a reusable asynchronous TCP network library based on standalone Asio. It it designed to be shared by both game client and server applications.

## 2. Design Goals

 - Client and Server share the same Session implementation
 - No game-specific logic in the library
 - Testable without game code

## 3. Non-Goals

 - This library does not interpret packet semantics
 - This library does not manage game entities
 - This library does not provide reliability beyond TCP

## 4. Architecture Overview

Application
 ├─ ISessionListener
 └─ NetworkService
     ├─ ClientService / ServerService
     ├─ Acceptor (server only)
     ├─ Session
     │  ├─ PacketCodec
     |  │  └─ PacketHeader
     │  ├─ RecvBuffer
     │  └─ SendBuffer
     └─ SessionManager

## 5. Threading Model

This library operates on a single Asio io_context, which may be executed by one or more network worker threads.

All Session methods are serialized using asio::strand.
SessionManager operations are also serialized through a dedicated strand owned by the NetworkService.

No internal data structure in this library is thread-safe.
Thread safety is achieved exclusively through task posting and strand-based serialization.

Application threads must not directly access Session or SessionManager instances. 
All interactions across threads must be performed by posting tasks to the appropriate strand.

## 6. Core Components

### NetworkService

**Role**
NetworkService owns Asio io_context and manages the lifetime of all network-related components

**Responsibilities**
 - Owns and runs a single io_context
 - Manage network worker threads
 - Owns the SessionManager and its lifetime
 - Provide a common base for client and server services

**Non-Responsibilities**
 - Creating application-level sessions
 - Interpreting packet semantics
 - Managing application logic

### ClientService / ServerService

**Role**
ClientService and ServerService define TCP connection policies for outgoing and incoming connections respectively

**Responsibilities**
 - Defines connection policies for outbound TCP connections (ClientService)
 - Defines acceptance policies for inbound TCP connections (ServerService)
 - Create Session instances from successfully connected sockets

**Non-Responsibilities**
 - Managing session lifetime after creation
 - Interpreting packet data
 - Handling application logic
 - Performing application-level synchronization

### Acceptor (Server only)

**Role**
Acceptor abstracts the asynchronous accept loop for server-side connections

**Responsibilities**
 - Own an acceptor socket
 - Accept incoming connections asynchronously
 - Delegate accepted sockets to ServerService

**Non-Responsibilities**
 - Session creation logic
 - Session management or storage
 - Error recovery policies

### Session

**Role**
Session represents a single TCP connection and acts as the core unit of communication

**Responsibilities**
 - Own a TCP socket and associated strand
 - Perform asynchronous read and write operations
 - Manage connection lifetime and shutdown
 - Deliver completed packets to the application via callbacks

**Non-Responsibilities**
 - Packet interpretation or application logic
 - Thread synchronization outside its strand

### SessionManager

**Role**
SessionManager tracks active Session instances within the service

**Responsibilities**
 - Add and remove active sessions
 - Provide session lookup by identifier
 - Coordinate bulk operations such as shutdown

**Non-Responsibilities**
 - Performing network I/O
 - Making application-level decisions

### ISessionListener

**Role**
ISessionListener defines the callback interface used to deliver session events to the application

**Responsibilities**
 - Receive connection, packet, and disconnection events
 - Act as the integration boundary with application logic

**Non-Responsibilities**
 - Performing network operations
 - Managing session lifetime
 - Synchronizing threads

### RecvBuffer / SendBuffer
RecvBuffer and SendBuffer are implementation-specific components.
Their internal data structures may change for performance reasons without affecting the public behavior of Session.

## 7. Packet Model



## 8. Session LifeCycle



## 9. Error & Shutdown Policy



## 10. Build & Intergration



## 11. Testing Strategy


