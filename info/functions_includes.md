### See markdown in VScode: `Ctrl+K V`


You must write a HTTP server in C++ 98.
Your executable will be run as follows:
./webserv [configuration file]

---

### **1. Process Management and Execution**
- **Functions**: `execve`, `fork`, `waitpid`, `kill`, `signal`
- **Includes**:  
  ```cpp
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <signal.h>
  ```
- **Explanations**:  
  - **`execve`**: Replaces the current process image with a new one, executing a program specified by a file path. Useful for running external programs.
  - **`fork`**: Creates a new process (child) as a copy of the current process. Essential for handling multiple client connections.
  - **`waitpid`**: Waits for a specific child process to terminate. Used to clean up resources after child processes finish.
  - **`kill`**: Sends a signal to a process (e.g., terminate a process). Can be used for managing child processes.
  - **`signal`**: Sets up custom handlers for signals (e.g., SIGINT or SIGCHLD).

---

### **2. File and Directory Operations**
- **Functions**: `access`, `stat`, `open`, `opendir`, `readdir`, `closedir`, `chdir`, `read`, `write`, `close`
- **Includes**:  
  ```cpp
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <dirent.h>
  ```
- **Explanations**:  
  - **`int access(const char *pathname, int mode)`**: Checks the file's accessibility (e.g., read/write permissions).
  - **`stat`**: Retrieves file information like size, modification time, etc.
  - **`open`**: Opens a file for reading/writing. Returns a file descriptor.
  - **`opendir`**, **`readdir`**, **`closedir`**: Work with directories. For example, `opendir` opens a directory, `readdir` reads entries, and `closedir` closes the directory.
  - **`chdir`**: Changes the current working directory of the process.
  - **`read`**, **`write`**, **`close`**: Read from, write to, and close file descriptors.

---

### **3. Networking**
- **Functions**: `socket`, `accept`, `listen`, `bind`, `connect`, `send`, `recv`, `setsockopt`, `getsockname`, `getaddrinfo`, `freeaddrinfo`, `socketpair`
- **Includes**:  
  ```cpp
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  ```
- **Explanations**:  
  - **`socket`**: Creates a socket for network communication.
  - **`accept`**: Accepts a new connection on a listening socket.
  - **`listen`**: Marks a socket as a listening socket.
  - **`bind`**: Binds a socket to an address/port.
  - **`connect`**: Establishes a connection to a remote server.
  - **`send`**, **`recv`**: Send and receive data over a socket.
  - **`setsockopt`**: Configures options for a socket (e.g., timeout, reuse address).
  - **`getsockname`**: Retrieves the local address/port associated with a socket.
  - **`getaddrinfo`**, **`freeaddrinfo`**: Used for address resolution (e.g., converting a hostname to an IP address).
  - **`socketpair`**: Creates a pair of connected sockets (useful for IPC).

---

### **4. Data Conversion and Error Handling**
- **Functions**: `htons`, `htonl`, `ntohs`, `ntohl`, `strerror`, `gai_strerror`, `errno`
- **Includes**:  
  ```cpp
  #include <arpa/inet.h>
  #include <string.h>
  #include <errno.h>
  ```
- **Explanations**:  
  - **`htons`**, **`htonl`**, **`ntohs`**, **`ntohl`**: Convert integers between host and network byte order.
  - **`strerror`**: Converts an error code (`errno`) to a human-readable string.
  - **`gai_strerror`**: Converts `getaddrinfo` error codes to a string.
  - **`errno`**: A global variable set when system calls encounter errors.

---

### **5. Multiplexing**
- **Functions**: `select`, `poll`, `epoll_create`, `epoll_ctl`, `epoll_wait`, `kqueue`, `kevent`
- **Includes**:  
  ```cpp
  #include <sys/select.h>
  #include <poll.h>
  #include <sys/epoll.h>
  #include <sys/event.h>
  ```
- **Explanations**:  
  - **`select`**: Monitors multiple file descriptors for readiness. Useful for basic multiplexing.
  - **`poll`**: Similar to `select`, but more scalable for larger descriptor sets.
  - **`epoll_create`**, **`epoll_ctl`**, **`epoll_wait`**: Advanced and efficient I/O multiplexing for Linux.
  - **`kqueue`**, **`kevent`**: Multiplexing mechanism for BSD systems.

---

### **6. File Descriptor Operations**
- **Functions**: `dup`, `dup2`, `fcntl`, `pipe`, `close`
- **Includes**:  
  ```cpp
  #include <unistd.h>
  #include <fcntl.h>
  ```
- **Explanations**:  
  - **`dup`**, **`dup2`**: Duplicate file descriptors, often used for redirection.
  - **`fcntl`**: Configures file descriptors (e.g., set non-blocking mode).
  - **`pipe`**: Creates a unidirectional data channel between two processes.
  - **`close`**: Closes a file descriptor to free resources.

---

### **Use Case Summary**
- **Networking**: `socket`, `accept`, `bind`, `listen`, `connect`, `send`, `recv`.
- **Concurrency**: `fork`, `execve`, `waitpid`, `select`, `poll`, `epoll`, `kqueue`.
- **Error Handling**: `strerror`, `gai_strerror`, `errno`.
- **File I/O**: `open`, `read`, `write`, `stat`, `opendir`, `readdir`.
- **Process Communication**: `pipe`, `socketpair`.
- **Signal Handling**: `signal`, `kill`.

---
---

### **Example Workflow in a Server:**

1. **Setup the server:**
   - Create a socket with `socket()`.
   - Bind it to an address and port with `bind()`.
   - Start listening for connections with `listen()`.

2. **Main loop:**
   - Accept connections using `accept()`.
   - Receive requests with `recv()` or `read()`.
   - Process the request (e.g., serve a file, handle API logic).
   - Send responses using `send()` or `write()`.

3. **Cleanup:**
   - Close client sockets with `close()` after handling requests.
   - Close the server socket when shutting down.
