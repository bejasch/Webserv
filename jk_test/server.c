// Server side C program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_EVENTS 10

struct http_request
{
    char *method;
    char *path;
    char *version;
};

struct http_response
{
    char *status;
    char *content_type;
    char *content_length;
    char *body;
};

struct http_request *parse_http_request(char *buffer)
{
    struct http_request *request = malloc(sizeof(struct http_request));
    char *token = strtok(buffer, " ");
    request->method = token;
    token = strtok(NULL, " ");
    request->path = token;
    token = strtok(NULL, "\r\n");
    request->version = token;
    printf("Method: %s\n", request->method);
    printf("Path: %s\n", request->path);
    printf("Version: %s\n", request->version);
    return request;
}

struct http_response *handle_request(struct http_request *request)
{
    struct http_response *response = malloc(sizeof(struct http_response));
    if (strcmp(request->method, "GET") == 0)
    {
        //here based on the path we should check if the file exists or not (basically check file name in the directory)
        // if the file exists then we should read the file to the response struct and return it
        if (strcmp(request->path, "/") == 0)
        {
            response->status = "HTTP/1.1 200 OK";
            response->content_type = "Content-Type: text/plain";
            response->content_length = "Content-Length: 12";
            response->body = "Hello world!";
            return response;
        }
        else if (strcmp(request->path, "/index.html") == 0)
        {
            response->status = "HTTP/1.1 200 OK";
            response->content_type = "Content-Type: text/html";
            response->content_length = "Content-Length: 15";
            response->body = "<h1>Hello world!</h1>";
            return response;
        }
        else if (strcmp(request->path, "/info.html") == 0)
        {
            response->status = "HTTP/1.1 200 OK";
            response->content_type = "Content-Type: text/html";
            response->content_length = "Content-Length: 13";
            response->body = "<h1>Info Page!</h1>";
            return response;
        }
        else
        {
            response->status = "HTTP/1.1 404 Not Found";
            response->content_type = "Content-Type: text/plain";
            response->content_length = "Content-Length: 9";
            response->body = "Not found!";
            return response;
        }
    }
    else
    {
        response->status = "HTTP/1.1 405 Method Not Allowed";
        response->content_type = "Content-Type: text/plain";
        response->content_length = "Content-Length: 18";
        response->body = "Method not allowed!";
        return response;
    }
    return response;
}

void write_response(int new_socket, struct http_response *response)
{
    char *response_string = malloc(strlen(response->status) + strlen(response->content_type) + strlen(response->content_length) + strlen(response->body) + 4);
    // sprintf writes formatted text to an output string -> response_string
    sprintf(response_string, "%s\n%s\n%s\n\n%s", response->status, response->content_type, response->content_length, response->body);
    write(new_socket, response_string, strlen(response_string));
    free(response_string);
}


int main(int argc, char const *argv[])
{
    int server_fd, new_socket, epoll_fd, n, i;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    struct epoll_event ev, events[MAX_EVENTS];
    struct http_request *request;
    struct http_response *response;
    
    // Creating the file descriptor of the program running the server
    server_fd = socket(AF_INET, SOCK_STREAM, 0); //

    address.sin_family = AF_INET; // address family used previously
    address.sin_addr.s_addr = INADDR_ANY; // this is my IP address
    address.sin_port = htons( PORT ); // the port I would like to expose
    bind(server_fd, (struct sockaddr *)&address, sizeof(address)); // bind server file descriptor to socket address

    // Make server_fd non-blocking
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    listen(server_fd, 10); // 10 defines how many pending connections can be queued before connections are refused.


    // Create epoll instance
    epoll_fd = epoll_create1(0);
    // Add server_fd to epoll instance to monitor incoming connections
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    while (1)
    {
        n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); // every time there is a new connection, an event is triggered, and n increases by 1
        // Loop through the events
        for (i = 0; i < n; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                // server_fd is the listening socket, thus we need to create a new socket for the communication with the client. This socket is used for communication.
                new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                // Make the new socket non-blocking
                fcntl(new_socket, F_SETFL, O_NONBLOCK);
                // Add new_socket to epoll instance to monitor read events
                ev.events = EPOLLIN; // Event for reading data
                ev.data.fd = new_socket;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev); ///adds `new_socket` to epoll instance and watch it for EPOLLIN
            }
            if (events[i].events & EPOLLIN)
            {
                // Data is available to read from the socket
                char buffer[30000] = {0};
                int valread = read(events[i].data.fd, buffer, sizeof(buffer));

                if (valread == 0)
                {
                    printf("Connection closed by client.\n");
                    close(events[i].data.fd);
                    continue;
                }
                else if (valread == -1)
                {
                    printf("Error reading from socket.\n");
                    close(events[i].data.fd);
                    continue;
                }
                else if (valread > 0)
                {
                    // Process the HTTP request and generate a response
                    printf("HTTP Request: \n\n\n%s\n\n\n", buffer);
                    request = parse_http_request(buffer);
                    response = handle_request(request);
                    write_response(events[i].data.fd, response);
                    free(request);
                    free(response);

                    // Close the socket after handling the request
                    close(events[i].data.fd);
                }
            }
        }
    }
    close(server_fd);
    return 0;
}
