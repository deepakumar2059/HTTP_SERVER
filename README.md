# HTTP_SERVER

A lightweight HTTP/1.1 static file server written in C++ for Linux. The project demonstrates how to build a web server using low-level POSIX networking APIs, non-blocking sockets, Linux `epoll`, HTTP request parsing, response streaming, and basic server logging.

The server listens on port `8080`, accepts multiple client connections, parses incoming HTTP requests, and serves files from the `static_files` directory. It includes example HTML pages, CSS styling, an image, and a form for testing browser and POST requests.

> **Project status:** This is an educational HTTP server implementation. It demonstrates core server concepts but is not intended to replace production web servers such as Nginx, Apache, or Caddy.

---

## Table of Contents

- [Features](#features)
- [Project Structure](#project-structure)
- [How the Server Works](#how-the-server-works)
- [Requirements](#requirements)
- [Build and Run](#build-and-run)
- [Using the Server](#using-the-server)
- [HTTP Request Parsing](#http-request-parsing)
- [Response Handling](#response-handling)
- [Logging](#logging)
- [Static Website](#static-website)
- [Testing](#testing)
- [Known Limitations](#known-limitations)
- [Future Improvements](#future-improvements)
- [Learning Goals](#learning-goals)
- [License](#license)

---

## Features

- TCP socket creation using POSIX socket APIs
- Binding to a configurable server port
- Listening for incoming TCP connections
- Non-blocking client and listener sockets
- Linux `epoll`-based event loop
- Multiple simultaneous client connections
- Incremental HTTP request parsing
- Support for request lines, headers, and request bodies
- `Content-Length` handling for request bodies
- Static file serving
- Automatic default routing from `/` to `/index.html`
- MIME type detection for common file extensions
- HTTP status responses for:
  - `200 OK`
  - `404 NOT FOUND`
  - `500 INTERNAL SERVER ERROR`
- Partial-write handling for non-blocking sockets
- File transmission using Linux `sendfile`
- Basic request and connection logging
- Example HTML, CSS, image, and form files

---

## Project Structure

```text
HTTP_SERVER/
├── README.md
├── src/
│   ├── main.cpp
│   ├── makefile
│   ├── a.out
│   │
│   ├── http/
│   │   ├── httpParser.h
│   │   ├── httpParser.cpp
│   │   ├── threadPool.h
│   │   └── threadPool.cpp
│   │
│   ├── network/
│   │   ├── socket.h
│   │   └── socket.cpp
│   │
│   ├── server/
│   │   ├── server.h
│   │   ├── server.cpp
│   │   ├── connections.h
│   │   └── connections.cpp
│   │
│   └── log/
│       ├── logger.h
│       ├── log.cpp
│       └── log
│
├── static_files/
│   ├── index.html
│   ├── second.html
│   ├── form.html
│   ├── style.css
│   └── deepak.jpg
│
└── tests/
    ├── tempCodeRunnerFile.cpp
    ├── try
    └── try.h
```

### Source modules

#### `src/main.cpp`

This is the application entry point. It creates a `Server` object using port `8080` and starts the server:

```cpp
Server server(8080);
server.start();
```

#### `src/network/`

The `Socket` class provides a small wrapper around the server socket lifecycle:

1. Create a TCP socket.
2. Enable address reuse.
3. Bind the socket to `INADDR_ANY`.
4. Begin listening.
5. Accept incoming connections.

The listening backlog is configured as `10`.

#### `src/server/`

The server module contains the main event loop and connection management.

`server.cpp` uses Linux `epoll` to monitor:

- The listening socket for new clients.
- Client sockets for incoming data.
- Client sockets for writable events.
- Error and hang-up events.

Each active client is represented by a `Connections` object.

#### `src/http/`

This directory contains HTTP parsing and thread-pool-related code.

`httpParser.cpp` implements a state machine with these states:

```text
REQUEST_LINE → HEADERS → BODY → COMPLETE
```

The parser reads data from a `Buffer` and waits until enough bytes are available to complete a request.

`threadPool.cpp` implements a worker queue using:

- `std::thread`
- `std::queue`
- `std::mutex`
- `std::condition_variable`

The current server event loop does not instantiate the thread pool.

#### `src/log/`

The logger prints timestamped messages with one of three levels:

- `INFO`
- `ERROR`
- `DEBUG`

Examples of logged events include server startup, new client connections, client disconnections, and received requests.

#### `static_files/`

This directory is the document root served by the application. The server maps HTTP resources to files using:

```text
../static_files/<requested-resource>
```

#### `tests/`

The current test directory contains small experimental files and compiled artifacts. It does not currently contain an automated test suite.

---

## How the Server Works

The complete request flow is:

```text
Client
  │
  ▼
Listening TCP socket
  │
  ▼
accept()
  │
  ▼
Non-blocking client socket
  │
  ▼
epoll_wait()
  │
  ▼
Connections::handle_read()
  │
  ▼
Parser::consume()
  │
  ▼
Connections::handler_response_builder()
  │
  ▼
Connections::handle_write()
  │
  ├── Send HTTP headers
  └── Stream file with sendfile()
```

### Server initialization

When the program starts:

1. `main.cpp` creates a `Server` on port `8080`.
2. `Server::start()` creates a `Socket`.
3. The socket is created using `AF_INET` and `SOCK_STREAM`.
4. The socket is bound to all local interfaces.
5. The socket begins listening.
6. The listener is changed to non-blocking mode.
7. An `epoll` instance is created.
8. The listening socket is registered for `EPOLLIN`.

### Accepting clients

When the listening socket becomes readable, the server repeatedly calls `accept()` until no more pending connections are available.

Each accepted client socket is:

- Set to non-blocking mode.
- Stored in the `connections` map.
- Registered with `epoll`.
- Monitored for input, errors, and hang-ups.

### Reading requests

When a client socket receives data, `Connections::handle_read()` calls `recv()` into a temporary buffer.

The bytes are appended to the connection's `Buffer`. The parser consumes complete lines and body bytes as they become available.

This allows the server to handle requests that arrive in multiple network packets.

### Writing responses

Once a request is complete, a `Response` object is added to the connection's response queue.

The connection changes from `READING` to `WRITING`. The server then modifies the `epoll` event registration so the client is monitored for `EPOLLOUT`.

Responses are sent in two stages:

1. HTTP response headers are sent using `send()`.
2. The requested file is sent using Linux `sendfile()`.

If the socket cannot currently accept more data, the response remains queued and writing resumes when the socket becomes writable again.

---

## Requirements

The server uses Linux-specific APIs and should be run on a Linux environment.

### Required software

- Linux operating system
- GNU C++ compiler
- C++11 or newer support
- POSIX socket support
- Linux `epoll`
- Linux `sendfile`

Ubuntu or Debian users can install the compiler with:

```bash
sudo apt update
sudo apt install build-essential
```

The project is not currently configured with a complete build system. The existing `src/makefile` is empty, so compilation must currently be performed manually unless a Makefile is added.

---

## Build and Run

Clone the repository:

```bash
git clone https://github.com/deepakumar2059/HTTP_SERVER.git
cd HTTP_SERVER
```

Compile the active server implementation:

```bash
g++ -std=c++17 \
    src/main.cpp \
    src/server/server.cpp \
    src/server/connections.cpp \
    src/network/socket.cpp \
    src/log/log.cpp \
    -o http_server
```

Run the server from the `src` directory:

```bash
cd src
../http_server
```

Running from `src` is important because the static file handler uses this relative path:

```text
../static_files
```

The server should print a startup message similar to:

```text
[<timestamp>] [INFO] Server started on port 8080
```

Open the server in a browser:

```text
http://localhost:8080/
```

Stop the server with:

```text
Ctrl+C
```

### Alternative output location

You can also place the executable inside `src`:

```bash
g++ -std=c++17 \
    src/main.cpp \
    src/server/server.cpp \
    src/server/connections.cpp \
    src/network/socket.cpp \
    src/log/log.cpp \
    -o src/http_server
```

Then run:

```bash
cd src
./http_server
```

---

## Using the Server

### Home page

Request:

```http
GET / HTTP/1.1
Host: localhost:8080
```

The root path is internally converted to:

```text
/index.html
```

The server returns `static_files/index.html`.

### Available static resources

| URL | File |
|---|---|
| `/` | `static_files/index.html` |
| `/index.html` | `static_files/index.html` |
| `/second.html` | `static_files/second.html` |
| `/form.html` | `static_files/form.html` |
| `/style.css` | `static_files/style.css` |
| `/deepak.jpg` | `static_files/deepak.jpg` |

### Test with `curl`

Start the server, then use another terminal:

```bash
curl -i http://localhost:8080/
```

Request another page:

```bash
curl -i http://localhost:8080/second.html
```

Request CSS:

```bash
curl -i http://localhost:8080/style.css
```

Request the image:

```bash
curl -i http://localhost:8080/deepak.jpg
```

Request a file that does not exist:

```bash
curl -i http://localhost:8080/missing.html
```

The server should return a `404 NOT FOUND` response.

---

## HTTP Request Parsing

The parser is implemented in `src/http/httpParser.h` and `src/http/httpParser.cpp`.

A request is represented by the `Request` class:

```cpp
class Request {
public:
    string method;
    string resource;
    string version;
    unordered_map<string, string> headers;
    string body;
};
```

The parser recognizes:

- HTTP method
- Requested resource
- HTTP version
- Headers
- Request body

The parser uses `\r\n` as the line delimiter. It first reads the request line, then reads headers until it reaches an empty line.

If a `Content-Length` header exists, the parser waits until the complete body is available.

Example:

```text
POST /login HTTP/1.1
Host: localhost:8080
Content-Length: 27

username=deepak&password=123
```

The parser stores the body as a string and returns `Parser::COMPLETE`.

The `Buffer` class supports incremental reads through:

- `append()`
- `hasLine()`
- `getLine()`
- `hasBytes()`
- `getBytes()`

This design prevents the server from assuming that a complete HTTP request arrives in a single `recv()` call.

---

## Response Handling

Response creation is implemented in:

```text
src/server/connections.cpp
```

The response builder:

1. Reads the requested resource.
2. Converts `/` to `/index.html`.
3. Determines a MIME type.
4. Opens the requested file.
5. Reads its size with `fstat()`.
6. Builds HTTP response headers.
7. Adds the response to the connection queue.

Supported MIME types include:

| Extension | Content-Type |
|---|---|
| `.html` | `text/html` |
| `.css` | `text/css` |
| `.js` | `text/javascript` |
| `.json` | `application/json` |
| `.png` | `image/png` |
| Other files | `text/plain` |

Successful responses include:

```http
HTTP/1.1 200 OK
Content-Length: <file-size>
Content-Type: <mime-type>
Connection: keep-alive
```

Missing files produce:

```http
HTTP/1.1 404 NOT FOUND
Content-Type: text/html
Content-Length: 0
Connection: keep-alive
```

The server also generates a `500 INTERNAL SERVER ERROR` response if it cannot inspect an opened file using `fstat()`.

---

## Logging

Logging is provided by the `Logger` class in `src/log/logger.h`.

Supported log levels are:

```cpp
enum LogLevel {
    INFO,
    ERROR,
    DEBUG
};
```

Log messages include:

- Current time
- Log level
- Message text

Examples of events logged by the server:

```text
Server started on port 8080
New connection fd=5
Request: GET /index.html fd=5
Client disconnected fd=5
recv failed fd=5
```

Logging is currently written to standard output using `std::cout`.

---

## Static Website

The repository includes a small static website for testing.

### `index.html`

The default page contains:

- A heading
- A link to `second.html`
- An image reference to `deepak.jpg`
- A link to `form.html`

### `second.html`

This page is a portfolio-style page containing:

- A navigation header
- Hero section
- About section
- Skills section
- Projects section
- Contact section

It references `style.css`.

### `style.css`

The stylesheet provides:

- Responsive layouts
- CSS variables
- Navigation styling
- Hero section styling
- Cards and project grids
- Mobile breakpoints
- Buttons and contact section styling

### `form.html`

The form sends a POST request to:

```text
http://localhost:8080/login
```

The form includes:

- Username input
- Password input
- Submit button

The current server parses POST request bodies but does not implement a complete `/login` application route. Therefore, the form is mainly useful for testing request parsing and body handling.

---

## Testing

The repository currently contains experimental test files rather than an automated test framework.

You can manually test the server using:

```bash
curl -i http://localhost:8080/
curl -i http://localhost:8080/index.html
curl -i http://localhost:8080/second.html
curl -i http://localhost:8080/style.css
curl -i http://localhost:8080/missing.html
```

Test a POST request body:

```bash
curl -i \
  -X POST \
  -d "username=deepak&password=test" \
  http://localhost:8080/login
```

You can also open the pages in a browser and inspect the server logs while navigating between resources.

---

## Known Limitations

This project is designed for learning and experimentation. The following limitations are currently present:

1. The project does not include a working build script. `src/makefile` is empty.
2. The server is Linux-specific because it uses `epoll` and `sendfile`.
3. There is no graceful shutdown mechanism.
4. The event loop runs continuously until the process is terminated.
5. There is no request timeout handling.
6. HTTP methods are not fully validated.
7. HTTP versions are not validated.
8. Header parsing expects the format `Header: value`.
9. Chunked transfer encoding is not supported.
10. There is no URL decoding.
11. There is no path normalization or traversal protection.
12. A malicious path could potentially access unintended filesystem locations.
13. There is no range request support.
14. There is no TLS or HTTPS support.
15. There is no virtual-host support.
16. There is no access-control or authentication layer.
17. Error handling for system calls is incomplete in several places.
18. The thread pool code is not integrated into the main server.
19. The thread-pool implementation includes `handler.h`, which is not present in the current repository tree.
20. The repository contains compiled artifacts such as `a.out` and `tests/try`.
21. There is no automated unit or integration test suite.
22. The static file root depends on the directory from which the executable is started.
23. The POST form points to `/login`, but no dedicated login response handler is currently implemented.

---

## Future Improvements

Possible improvements include:

- Add a complete `Makefile`.
- Add compiler warnings such as `-Wall`, `-Wextra`, and `-pedantic`.
- Add include guards or `#pragma once` to header files.
- Add structured error handling for `socket`, `bind`, `listen`, `accept`, `epoll`, `open`, and `sendfile`.
- Use smart pointers instead of raw `Connections*` allocations.
- Add secure path normalization.
- Prevent directory traversal attacks.
- Implement proper HTTP status and method validation.
- Add support for `HEAD`.
- Add chunked transfer encoding.
- Add request and response size limits.
- Add connection timeouts.
- Add graceful shutdown using signals.
- Move MIME type handling into a dedicated module.
- Separate routing, response building, and file serving.
- Integrate the thread pool or remove unused thread-pool code.
- Add automated tests for the parser and server responses.
- Add continuous integration.
- Add HTTPS support through OpenSSL.
- Improve support for POST routes such as `/login`.
- Store logs in rotating files.
- Use a configurable document root and port.
- Remove generated binaries from the source repository.

---

## Learning Goals

This project is useful for learning how web servers work internally.

It demonstrates several important systems-programming concepts:

- TCP networking
- File descriptors
- Blocking and non-blocking I/O
- Event-driven programming
- `epoll`
- HTTP message structure
- Incremental parsing
- File descriptors for static content
- Partial writes
- Linux zero-copy file transmission
- Mutexes and condition variables
- Basic server observability through logging

The most important architectural idea is that the server does not create one blocking loop per client. Instead, it uses a central `epoll` event loop to monitor many file descriptors and processes each connection only when an event occurs.

---

## License

No license file is currently included in the repository.

If you plan to distribute or reuse this project, add a license such as MIT, Apache-2.0, or GPL-3.0 and update this section accordingly.

---

## Author

Created by [deepakumar2059](https://github.com/deepakumar2059).

Repository:

[https://github.com/deepakumar2059/HTTP_SERVER](https://github.com/deepakumar2059/HTTP_SERVER)
