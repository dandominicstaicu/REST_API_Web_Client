# HTTP Client REST API for Book Information Server

## 0. Introduction

The primary objective of this assignment was to develop a HTTP client to interact with a HTTP server that maintains a database of books. This involved handling various HTTP requests and managing stateless connections.

## 1. Main Logic

The core logic of the client and its API is implemented in the `client.c` file. Here are some key components:

- **Buffer Management**: A buffer is maintained for the message data, and two pointers initially set to `NULL` manage the values for the session's cookie and token.
- **TCP Connections**: Each command execution within the `while` loop initiates a new TCP connection to uphold the stateless nature of HTTP, ensuring that the outcome of the current request is independent of any previous ones.

## 2. Utility Functions

Utility functions are consolidated in the `functions.c` file, forming the solution's backbone. Each command function, such as `register`, `login`, and `logout`, corresponds to specific HTTP request types: GET, POST, and DELETE.

### Implementation Details:

- **HTTP Headers**: These are constructed using helper functions provided in `requests.c`, developed during a laboratory session.
- **JSON Handling**: Commands with data payloads utilize JSON objects, built using the Parson library for compatibility with the procedural programming style of C.
- **Response Parsing**: Server responses are parsed into strings. Functions like `strtok()` and `strchr()` are used extensively to extract tokens or cookies, necessary due to the limitations of the C programming language.

## 3. JSON Library - Parson

Given the project's adherence to procedural programming in C, the Parson library was chosen for JSON manipulation due to its simplicity and robust functionality.

### Reasons for Choosing Parson:

- **Procedural Compatibility**: Perfect fit for a project not using object-oriented programming.
- **Intuitive API**: Allows for straightforward creation and parsing of JSON objects with minimal code.
- **Open Source and Reliable**: Parson is well-documented, open-source, and considered bug-free, making it ideal for educational and developmental use.
