
#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

#include "AllHeaders.hpp"

class Server {
    private:
        // server-set
        int						server_fd;
        struct sockaddr_in		address;
		std::map<int, HttpReq>	client_requests;
		Config config;
        std::vector<Route> routes;
		ServerManager &server_manager;

    public:
		Server(ServerManager &server_manager);
		~Server();
		void setServer(const std::string& config_file);
		void setUpServer();
		int getServerFd();
		void acceptConnection(int epoll_fd);
		void handleRequest(int fd);

    private:
		void addRoute(const Route& route);
        void addServer(const Config& config);
};

#endif
