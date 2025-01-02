## Webserv

The project is about writing an own HTTP server. It can be tested with an actual browser.
The primary function of a web server is to store, process, and deliver web pages to clients. The communication between client and server takes place using the Hypertext
Transfer Protocol (HTTP).
Pages delivered are most frequently HTML documents, which may include images, style sheets, and scripts in addition to the text content. Multiple web servers may be used for a high-traffic website. A user agent, commonly a web browser or web crawler, initiates communication by requesting a specific resource using HTTP and the server responds with the content of that resource or an error message if unable to do so. The resource is typically a real file on the server’s secondary storage, but this is not necessarily the case and depends on how the webserver is implemented.
While the primary function is to serve content, full implementation of HTTP also
includes ways of receiving content from clients. This feature is used for submitting web forms, including the uploading of files.

## Features to implement:

1. Non-Blocking Servers: Decide between poll / epoll !

2. HTTP Method Support: **GET**, **POST** (modify underlaying data / create new recourses), **PUT** (update existing recource), **DELETE** (remove a specified resource), **HEAD** (only retrieves the headers of a resource, without the body)

-> CRUD : Create, read, update, delete (post, get, put, delete)

3. Server Configuration Options:
- **Port Configuration**: Users can specify the port on which the server listens for incoming connections.
- **Root Directory**: The server’s root directory can be configured, allowing users to serve files from any specified location on their system.
- **Error Pages**: Custom error pages can be set up for different HTTP error codes, improving the user experience by providing meaningful error messages.
- **Max Body Size**: Users can configure the maximum body size for incoming requests, helping to manage server load and security.
- **Allow methods**: Users can set different methods being allowed for different locations, as well as for the root itself.
- **Authorization**: An authorization can be set with the login and password as a security measure.
- **Redirection**: A 301 and 302 redirect can be set for a permanent and temproral change of locations.
- **Autoindex**: An autoindex can be turned on and off for setting default page display.
- **CGI management**: Different CGI extensions can be allowed or restricted for all the locations being set.
- **Server name display**: Server domain names are being stored and displayed for a better server management.

4. Logging and Monitoring: **Access Logs** (requests: method, URL, response status, and client IP address), **Error Logs** (server-side errors: failed requests and internal server errors), **Server creation Logs** (list of created/available servers)

5. **Static Content** (serves static files such as HTML, CSS, JavaScript, images with features like caching and compression) and **Dynamic Content** (interfere with backend scripts or applications written in languages like PHP, Python, or Bash)