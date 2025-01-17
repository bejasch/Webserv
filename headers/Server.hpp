
#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

#include "AllHeaders.hpp"

// TODO: Why is the Config as a pointer?
class Server {
    private:
        // server-set
        int						server_fd;
        struct sockaddr_in		address;
		std::map<int, HttpReq>	client_requests;
		Config *config;
		ServerManager &server_manager;

    public:
		Server(ServerManager &server_manager);
		~Server();
		void setServer(Config *config); //define server configs
		void setUpServer(); //define socket and bind to server fd
		Config *getConfig();
		int getServerFd();
		void acceptConnection(int epoll_fd);
		void handleRequest(int fd);
		Config *getConfig() { return config; };
};

#endif
