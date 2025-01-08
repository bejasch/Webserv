#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <iostream>

class Server {
    private:
        const static int MAX_EVENTS = 10;
        int server_fd, epoll_fd;
        struct sockaddr_in address;
        struct epoll_event ev, events[MAX_EVENTS];

    public:
        Server();
        ~Server();
        void start();
        void stop();
    
    private:
        void acceptConnection();
        void handleRequest(int fd);
};

#endif