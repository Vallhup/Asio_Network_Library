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

Asio_Network_Module
 ├─ NetworkService
 |  ├─ ClientService / ServerService
 |  └─ Acceptor (server only)
 |
 ├─ Connection
 |  ├─ asio::socket
 |  ├─ asio::strand
 |  ├─ async send / recv
 |  ├─ packet framing
 |  └─ IConnectionListener
 |
 └─ Packet (Header + Payload)

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

**Non-Responsibilities**
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
 - Connection creation logic
 - Connection management or storage
 - Error recovery policies

### IAcceptListener (Server only)

The network library does not provide a session management facility

The server application is responsible for implementing IAcceptListenerto process sockets accepted by the Acceptor and bind them to application-level Connection or session contexts

**Role**
Defines the server-side hook invoked when a new socket is accepted

**Responsibilities**
 - Receive ownership of accepted TCP sockets
 - Create Connection instances from accepted sockets
 - Bind Connections to application-level session or client contexts

**Non-Responsibilities**
 - Managing accept loops
 - Performing network I/O
 - Defining application session policies

### Connection

**Role**
Connection represents a single transport-level connection and abstracts all asynchronous network I/O behavior

**Responsibilities**
 - Owns a single asio::socket
 - Owns a dedicated strand and serializes all internal state transitions within that strand
 - Perfroms asynchronous network I/O
 - Assembles raw byte streams into packet units
 - Notifies packet events via IConnectionListener

**Non-Responsibilities**
 - Interpreting packet meaning
 - Managing player or session state
 - Managing application logic

### IConnectionListener

All applications that create and own Connection instances must provide an implementation of the IConnectionListener interface

**Role**
Defines the callback interface between the network layer and the application layer

**Responsibilities**
 - Receive connection lifecycle events
 - Receive completed packets
 - Bridge network events to application logic
 - Callbacks are invoked within the associated Connection's strand

**Non-Responsibilities**
 - Performing network I/O
 - Managing connection lifetime
 - Thread synchronization

### RecvBuffer / SendBuffer
RecvBuffer and SendBuffer are implementation-specific components.
Their internal data structures may change for performance reasons without affecting the public behavior of Session.

## 7. Packet Model



## 8. Session LifeCycle



## 9. Error & Shutdown Policy



## 10. Build & Intergration



## 11. Testing Strategy


