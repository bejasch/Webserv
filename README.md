# Webserv Key Requirements Checklist

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

### Multiple Servers
✅ **Done**: Handle multiple servers with several different ports. (Choose the port and host of each ’server’)

### Free Allocated Memory
✅ **Done**: JK

### Signal Handling
✅ **Done**: JK

### Configuration File Handling
✅ **Done**: Your program does not currently handle a configuration file, which should be passed as a command-line argument or use a default path. The configuration file might specify settings like ports to listen on, file paths, or other server settings.

### Serving Static Files
✅ **Done**: While you serve basic text responses, you do not yet handle serving actual static files (like HTML, CSS, JavaScript, images, etc.) from the filesystem based on the GET request path (e.g., serving `index.html` from disk).

### POST Method Support
✅ **Done**: Your server does not yet support POST requests. This would typically involve receiving data in the request body, processing it (such as saving a file or creating a resource), and returning a response.

### DELETE Method Support
✅ **Done**: The DELETE method is not yet implemented. It should allow clients to delete resources (like files) on the server.

### File Uploads
✅ **Done**: You do not currently handle file uploads with POST requests. This typically involves parsing multipart form data and saving the uploaded files to the server’s file system.

### Serve Fully Static Website
✅ **Done**: Your server does not yet serve a fully static website from files, meaning it does not serve actual files (HTML, images, etc.) from disk based on the URL path.

### Stress Testing / Performance Under Load
✅ **Done**: You will need to test your server’s performance under load, especially to ensure it stays available and responsive during high traffic using siege.

### Listening to Multiple Ports
✅ **Done**: Your server currently only listens on a single port (8080). You need to implement the ability to listen on multiple ports as specified in the configuration file.

### Error Handling and Logging
✅ **Done**: The program should include robust error handling, especially around file operations and socket interactions. You also need logging to help debug and monitor the server's operation.

### Response Headers for Dynamic Content (like Content-Length)
✅ **Done**: When serving actual files (especially larger files), you'll need to properly set headers like `Content-Length` for dynamic responses. You’re currently hardcoding `Content-Length` for static responses, but this will need to be calculated for real file content.

### CGI
✅ **Done**: JK
- GET request done, POST WIP

## Preparation of the Evaluation

### homebrew
1. Set up homebrew, direct packages to sgoingfre
```bash
#!/bin/zsh
# https://docs.brew.sh/Installation
set -e

git clone https://github.com/Homebrew/brew /sgoinfre/goinfre/Perso/$USER/homebrew --depth=1

echo 'export PATH=$PATH:/sgoinfre/goinfre/Perso/$USER/homebrew/bin' >> ~/.zshrc

cd /sgoinfre/goinfre/Perso/$USER


eval "$(homebrew/bin/brew shellenv)"
brew update --force --quiet
chmod -R go-w "$(brew --prefix)/share/zsh"

echo "Brew installed in /sgoinfre/goinfre/Perso/$USER/homebrew"
echo "Don't forget to source your .zshrc"
echo " "
echo "source ~/.zshrc"
```

2. Install siege package
`brew install siege`

### Useful Network Diagnostic Commands

Below is a list of useful network diagnostic and troubleshooting commands.

 1. `dig example.com`
   - Performs a DNS lookup for the given domain and provides detailed information about DNS records.

 2. `nslookup example.com`
   - A simple DNS query tool for resolving domain names to IP addresses.

 3. `curl -v http://127.0.0.1:8081/`
   - Sends an HTTP request and displays detailed information about the HTTP communication.

 4. `ping 127.0.0.1`
   - Tests network connectivity by sending ICMP echo requests to the specified IP address.
   - The ping command cannot be used with a specific port because it uses ICMP (Internet Control Message Protocol), not TCP or UDP, which are the protocols that involve ports.

 5. `telnet 127.0.0.1 8080`
   - Opens a Telnet connection to a specified IP address and port, useful for checking basic connectivity.

 6. `netstat -tulnp`
   - Displays active network connections, listening ports, and the associated processes.

 7. `curl --resolve example.com:8080:127.0.0.1 http://example.com/`
   - Resolves a domain to a specific IP address and port without modifying DNS records.

 8. `traceroute example.com` (only with sudo)
   - Traces the route packets take to reach the destination, showing each hop along the way.

 9. `ss -tulnp`
   - Displays socket statistics and information about listening ports and associated processes.

 10. `curl -I http://127.0.0.1:8081/`
   - Sends a HEAD request to retrieve only the HTTP headers from a server.

### Useful `curl` Commands

1. **`curl -I`**
   - Fetch the HTTP headers (response headers) of a URL.
   - Example:  
     ```bash
     curl -I http://127.0.0.1:8081/
     ```

2. **`curl -X`**
   - Specify an HTTP request method (GET, POST, PUT, DELETE, etc.)
   - Example:  
     ```bash
     curl -X POST http://127.0.0.1:8081/ -d "name=John&age=30"
     ```

3. **`curl -d`**
   - Send data in a POST request.
   - Example:  
     ```bash
     curl -X POST -d "name=John&age=30" http://127.0.0.1:8081/
     ```

4. **`curl -L`**
   - Follow redirects.
   - Example:  
     ```bash
     curl -L http://127.0.0.1:8081/
     ```

5. **`curl -v`**
   - Enable verbose mode to see the detailed process of the request.
   - Example:  
     ```bash
     curl -v http://127.0.0.1:8081/
     ```

6. **`curl -u`**
   - Send HTTP basic authentication credentials.
   - Example:  
     ```bash
     curl -u username:password http://127.0.0.1:8081/
     ```

7. **`curl -H`**
   - Send custom headers with the request.
   - Example:  
     ```bash
     curl -H "Content-Type: application/json" http://127.0.0.1:8081/
     ```

8. **`curl -X DELETE`**
   - Send a DELETE request.
   - Example:  
     ```bash
     curl -X DELETE http://127.0.0.1:8081/resource
     ```

9. **`curl -o`**
   - Save the response to a file.
   - Example:  
     ```bash
     curl -o output.html http://127.0.0.1:8081/
     ```

10. **`curl -T`**
    - Upload a file with a PUT request.
    - Example:  
      ```bash
      curl -T file.txt http://127.0.0.1:8081/upload
      ```

11. **`curl -F`**
    - Submit a form with a file upload using POST.
    - Example:  
      ```bash
      curl -F "file=@path/to/file" http://127.0.0.1:8081/upload
      ```

12. **`curl --resolve`**
    - Resolve a domain name to a specific IP address (useful for testing custom domains).
    - Example:  
      ```bash
      curl --resolve example.com:8080:127.0.0.1 http://example.com:8080/
      ```

13. **`curl -x`**
    - Use a proxy server for the request.
    - Example:  
      ```bash
      curl -x http://proxyserver:8080 http://127.0.0.1:8081/
      ```
