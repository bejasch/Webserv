# Key Requirements Checklist

Here is a breakdown of the requirements in your project, organized into what you’ve already implemented and what’s still missing.

## Already Implemented

### Non-blocking I/O
✅ You have used `fcntl()` to make both the server socket and client sockets non-blocking. This allows the server to not block while waiting for data from clients, ensuring the server stays responsive.

### Using `epoll()` for I/O Multiplexing
✅ Your server uses `epoll_wait()` to monitor multiple file descriptors, handling I/O events (readable sockets) without blocking. This is the equivalent of `poll()`, but using `epoll` is more efficient.

### Multiple Sockets and Events
✅ You are able to handle multiple incoming connections using `epoll` and add new sockets dynamically when a connection is accepted (`epoll_ctl()`).

### GET Method Support
✅ Your server supports GET requests and provides basic responses for different paths (/, /index.html, /info.html). This fulfills the basic HTTP request processing for serving static content.

### HTTP Status Codes
✅ You correctly return HTTP status codes like `200 OK`, `404 Not Found`, `405 Method Not Allowed` in your responses based on the request method and path.

### Default Error Pages
✅ Your server returns a default error page (`Not found!`) when the requested resource is not available, fulfilling the requirement for a default error page.

### Non-blocking File Descriptors for I/O
✅ You’re using non-blocking I/O in your `epoll` loop, which allows the server to handle multiple connections concurrently and respond quickly to requests.

---

## Missing / Pending Implementation

### Configuration File Handling
❌ **Missing**: Your program does not currently handle a configuration file, which should be passed as a command-line argument or use a default path. The configuration file might specify settings like ports to listen on, file paths, or other server settings.
https://www.youtube.com/watch?v=8P2r0xSXk28&ab_channel=TonyTeachesTech
https://www.youtube.com/watch?v=MQpw4oXjIqY&ab_channel=KacperSzurekEN
https://www.youtube.com/watch?v=C5kMgshNc6g&ab_channel=NGINX

### Serving Static Files
❌ **Missing**: While you serve basic text responses, you do not yet handle serving actual static files (like HTML, CSS, JavaScript, images, etc.) from the filesystem based on the GET request path (e.g., serving `index.html` from disk).

### POST Method Support
❌ **Missing**: Your server does not yet support POST requests. This would typically involve receiving data in the request body, processing it (such as saving a file or creating a resource), and returning a response.

### DELETE Method Support
❌ **Missing**: The DELETE method is not yet implemented. It should allow clients to delete resources (like files) on the server.

### File Uploads
❌ **Missing**: You do not currently handle file uploads with POST requests. This typically involves parsing multipart form data and saving the uploaded files to the server’s file system.

### Serve Fully Static Website
❌ **Missing**: Your server does not yet serve a fully static website from files, meaning it does not serve actual files (HTML, images, etc.) from disk based on the URL path.

### Stress Testing / Performance Under Load
❌ **Missing**: You will need to test your server’s performance under load, especially to ensure it stays available and responsive during high traffic.
siege
locust


config;
cout << config << endl;
RDP;
STL; std::map<>, std::pair<>, std::vec<>, 

CLF common log format

### Listening to Multiple Ports
❌ **Missing**: Your server currently only listens on a single port (8080). You need to implement the ability to listen on multiple ports as specified in the configuration file.

### Error Handling and Logging
❌ **Missing**: The program should include robust error handling, especially around file operations and socket interactions. You also need logging to help debug and monitor the server's operation.

### Response Headers for Dynamic Content (like Content-Length)
❌ **Missing**: When serving actual files (especially larger files), you'll need to properly set headers like `Content-Length` for dynamic responses. You’re currently hardcoding `Content-Length` for static responses, but this will need to be calculated for real file content.

### CGI
❌ **Missing**: JK

### Multiple Servers
❌ **Missing**: Handle multiple servers with several different ports. (Choose the port and host of each ’server’)

### Free Allocated Memory
❌ **Missing**: JK

### Signal Handling
❌ **Missing**: JK



## Eval Checks

### 1. check for read and write
The select() (or equivalent) should be in the main loop and should check file descriptors for read and write AT THE SAME TIME. If not, the grade is 0 and the evaluation process ends now.

```c
void Server::acceptConnection(int epoll_fd) {
    // ... existing code ...
    // Add new_socket to epoll instance to monitor both read and write events
    ev.events = EPOLLIN | EPOLLOUT; // Monitor both read and write
    ev.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
        perror("Failed to add client_fd to epoll");
        close(client_fd);
        return;
    }

void ServerManager::startServers() {
    // ... existing code ...

    for (int i = 0; i < servers.size(); ++i) {
        Server *server = servers[i];
        int server_fd = server->getServerFd();
        epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT;  // Monitor both read and write
        ev.data.fd = server_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
            perror("Failed to add server_fd to epoll");
            return;
        }
    }
    handleEvents();
}

void ServerManager::dispatchEvent(const epoll_event& event) {
    // Handle server socket events
    for (int i = 0; i < servers.size(); ++i) {
        Server *server = servers[i];
        if (event.data.fd == server->getServerFd()) {
            if (event.events & EPOLLIN) {
                server->acceptConnection(epoll_fd);
            }
            return;
        }
    }

    // Handle client socket events
    for (std::map<int, Server*>::iterator it = clientfd_to_serverfd.begin(); 
         it != clientfd_to_serverfd.end(); ++it) {
        if (event.data.fd == it->first) {
            Server *server = it->second;
            if (event.events & EPOLLIN) {
                server->handleRequest(event.data.fd);
            }
            if (event.events & EPOLLOUT) {
                server->handleWrite(event.data.fd);
            }
            return;
        }
    }
    
    std::cerr << "Unknown fd: " << event.data.fd << std::endl;
}
```


With these changes, your server will properly monitor both read and write events simultaneously in the main loop, which should satisfy this requirement. The epoll_wait function will return when any of the monitored file descriptors are ready for either reading or writing.

### 2. one read or write per client
There should be only one read or one write per client per select() (or equivalent). Ask the group to show you the code from the select() (or equivalent) to the read and write of a client.

```c
void Server::handleRequest(int client_fd) {
    HttpReq &request = client_requests[client_fd];
    HttpRes httpResponse;

    // 3. Single read operation per event
    char buffer[30000] = {0};
    int valread = read(client_fd, buffer, sizeof(buffer));
    
    if (valread == 0) {
        // ... error handling ...
    }

    // 4. Process the request and immediately write response
    if (request.processData(std::string(buffer, valread))) {
        httpResponse.handleRequest(request, *this);
        httpResponse.writeResponse(client_fd);  // <-- This is problematic!
        
        client_requests.erase(client_fd);
        close(client_fd);
    }
}
```

There's a potential issue here: The code performs a write operation (httpResponse.writeResponse) directly after processing the request, without going through epoll to check if the socket is ready for writing.
To fix this and ensure only one read/write per epoll event, you should modify the code like this:

```c
void Server::handleRequest(int client_fd) {
    HttpReq &request = client_requests[client_fd];

    // Single read operation
    char buffer[30000] = {0};
    int valread = read(client_fd, buffer, sizeof(buffer));
    
    if (valread <= 0) {
        // ... error handling ...
        return;
    }

    // Process the request
    if (request.processData(std::string(buffer, valread))) {
        // Prepare response
        HttpRes httpResponse;
        httpResponse.handleRequest(request, *this);
        
        // Store response and register for write event
        pending_responses[client_fd] = httpResponse;
        
        // Modify epoll to watch for write readiness
        epoll_event ev;
        ev.events = EPOLLOUT;
        ev.data.fd = client_fd;
        if (epoll_ctl(server_manager.getEpollFd(), EPOLL_CTL_MOD, client_fd, &ev) == -1) {
            perror("Failed to modify client_fd for writing");
            closeConnection(client_fd);
            return;
        }
    }
}

// New method to handle write events
void Server::handleWrite(int client_fd) {
    // Single write operation per event
    if (pending_responses.find(client_fd) != pending_responses.end()) {
        HttpRes &response = pending_responses[client_fd];
        
        // Attempt to write response
        ssize_t bytes_written = write(client_fd, response.getData(), response.getLength());
        
        if (bytes_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return; // Try again later
            }
            closeConnection(client_fd);
            return;
        }
        
        if (bytes_written == response.getLength()) {
            // Response fully sent
            pending_responses.erase(client_fd);
            
            if (!response.isKeepAlive()) {
                closeConnection(client_fd);
            } else {
                // Switch back to reading mode
                epoll_event ev;
                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(server_manager.getEpollFd(), EPOLL_CTL_MOD, client_fd, &ev);
            }
        }
    }
}
```

The key changes are:
- Each epoll event results in exactly one read OR one write operation
- Write operations are deferred until epoll indicates write readiness
- Responses are stored in a pending_responses map until they can be written
- The server properly transitions between read and write states using epoll_ctl

### 3. remove client upon error

```c
void Server::handleRequest(int client_fd) {
    // ... 
    int valread = read(client_fd, buffer, sizeof(buffer));

    if (valread == 0) {    // Client closed connection
        std::cerr << "Connection closed by client.\n";
        close(client_fd);
        client_requests.erase(client_fd); // Clean up state
        return;
    } else if (valread == -1) {
        std::cerr << "Error reading from socket.\n";
        close(client_fd);
        client_requests.erase(client_fd); // Clean up state
        return;
    }
```

Issues with current implementation:
- Only removes from client_requests, but not from clientfd_to_serverfd map
- Doesn't remove the fd from epoll
- Doesn't handle pending responses
- Here's how it should be handled:

```c
// Add this helper method for proper cleanup
void Server::closeConnection(int client_fd) {
    // Remove from epoll first
    epoll_ctl(server_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL);
    
    // Close the socket
    close(client_fd);
    
    // Clean up all related data structures
    client_requests.erase(client_fd);
    pending_responses.erase(client_fd);
    server_manager.clientfd_to_serverfd.erase(client_fd);
}

void Server::handleRequest(int client_fd) {
    HttpReq &request = client_requests[client_fd];
    char buffer[30000] = {0};
    
    ssize_t valread = read(client_fd, buffer, sizeof(buffer));
    if (valread <= 0) {
        std::cerr << "Socket error or connection closed: " 
                  << (valread == 0 ? "Client closed connection" : strerror(errno)) 
                  << std::endl;
        closeConnection(client_fd);
        return;
    }
    // ... rest of the code
}

void Server::handleWrite(int client_fd) {
    if (pending_responses.find(client_fd) == pending_responses.end()) {
        return;
    }

    HttpRes &response = pending_responses[client_fd];
    ssize_t bytes_written = write(client_fd, response.getData(), response.getLength());
    
    if (bytes_written <= 0) {
        std::cerr << "Write error: " << strerror(errno) << std::endl;
        closeConnection(client_fd);
        return;
    }
    // ... rest of the code
}

// In case of accept errors
void Server::acceptConnection(int epoll_fd) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("Failed to accept connection");
        return;  // No need to cleanup as connection wasn't established
    }
    // ... rest of the code
}
```

### 4. this if fine

- Search for all read/recv/write/send and check if the returned value is correctly checked (checking only -1 or 0 values is not enough, both should be checked). --> we check for both.

- If errno is checked after read/recv/write/send, the grade is 0 and the evaluation process ends now. --> we dont check for errno.

- Writing or reading ANY file descriptor without going through the select() (or equivalent) is strictly FORBIDDEN. --> all events go through epoll_wait.


### 5. Change server hostname

Setup multiple servers with different hostnames (use something like: curl --resolve example.com:80:127.0.0.1 http://example.com/).

Usually this is done by adding a mapping to /etc/hosts

### 6. Handle standard error pages.

These should be specified in the config, and then served via the response class.

### 7. Return pages

### 8. MAX_BODY_SIZE

Do we restrain max_body_size?
