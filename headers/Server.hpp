
#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

#include "AllHeaders.hpp"

class Server {
    private:
        const static int MAX_EVENTS = 10; // should this be a static const or set by config?

        int						server_fd, epoll_fd;
        struct sockaddr_in		address;
        struct epoll_event		ev, events[MAX_EVENTS];
		std::map<int, HttpReq>	client_requests;

		// Config	config;

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
