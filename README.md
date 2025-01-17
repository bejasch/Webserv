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