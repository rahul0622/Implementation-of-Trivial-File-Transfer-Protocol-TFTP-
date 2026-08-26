# TFTP Client-Server

## Project Brief

TFTP (Trivial File Transfer Protocol) is a simple file transfer protocol that uses UDP for communication between a client and a server. The main goal of this project is to implement a TFTP client-server application in C that supports file upload and download operations using Read Request (RRQ) and Write Request (WRQ) packets.

The project handles the exchange of DATA and ACK packets, maintains proper packet sequencing, and includes necessary error handling for conditions such as invalid requests, missing files, and transfer failures.

## Features

- File download using Read Request (RRQ)
- File upload using Write Request (WRQ)
- UDP-based client-server communication
- DATA packet transmission
- ACK packet handling
- Packet sequence management
- File operations using Linux system calls
- Error handling for invalid requests and missing files

## Technologies Used

- C Programming
- Linux
- UDP Socket Programming
- POSIX System Calls
- File I/O

## Concepts Covered

- Client-Server Architecture
- UDP Socket Programming
- Network Protocol Implementation
- Packet Creation and Parsing
- File Handling
- Process Communication
- Error Handling

## Project Structure

```text
TFTP-Client-Server/
│
├── client/
│   └── client.c
│
├── server/
│   └── server.c
│
├── include/
│   └── tftp.h
│
├── Makefile
│
└── README.md
