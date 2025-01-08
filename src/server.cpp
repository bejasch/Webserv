#include "../headers/server.hpp"

Server::Server() {
    // Creating the file descriptor of the program running the server
    server_fd = socket(AF_INET, SOCK_STREAM, 0); //

    address.sin_family = AF_INET; // address family used previously
    address.sin_addr.s_addr = INADDR_ANY; // this is my IP address
    address.sin_port = htons( 8080 ); // the port I would like to expose
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
}

Server::~Server() {
    std::cout << "Server destructor called" << std::endl;
}

void Server::start() {
    int n, i;
    while (true)
    {
        n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); // every time there is a new connection, an event is triggered, and n increases by 1
        // Loop through the events
        for (i = 0; i < n; i++)
        {
            if (events[i].data.fd == server_fd)
                acceptConnection();
            if (events[i].events & EPOLLIN)
                handleRequest(events[i].data.fd);
        }
    }
}

void Server::stop() {
    close(server_fd);
    close(epoll_fd);
}

void Server::acceptConnection() {
    int new_socket;
    // server_fd is the listening socket, thus we need to create a new socket for the communication with the client. This socket is used for communication.
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    // Make the new socket non-blocking
    fcntl(new_socket, F_SETFL, O_NONBLOCK);
    // Add new_socket to epoll instance to monitor read events
    ev.events = EPOLLIN; // Event for reading data
    ev.data.fd = new_socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev); ///adds `new_socket` to epoll instance and watch it for EPOLLIN
}

void Server::handleRequest(int client_fd) {
    // Data is available to read from the socket
    char buffer[30000] = {0};
    int valread = read(client_fd, buffer, sizeof(buffer));

    if (valread == 0)
    {
        printf("Connection closed by client.\n");
        close(client_fd);
    }
    else if (valread == -1)
    {
        printf("Error reading from socket.\n");
        close(client_fd);
    }
    else if (valread > 0)
    {
        // Process the HTTP request and generate a response
        printf("HTTP Request: \n\n\n%s\n\n\n", buffer);
        request = parse_http_request(buffer);
        response = handle_request(request);
        write_response(client_fd, response);
        free(request);
        free(response);
        // Close the socket after handling the request
        close(client_fd);
    }
}
