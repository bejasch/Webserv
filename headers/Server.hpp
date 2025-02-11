#ifndef SERVER_HPP
#define SERVER_HPP

#include "AllHeaders.hpp"

class Server {
    private:
        // server-set
        int						server_fd;
        struct sockaddr_in		address;
		std::map<int, HttpReq>	client_requests;
		std::map<int, HttpRes>	pending_responses;
		Config					*config;
		ServerManager			&server_manager;

    public:
		Server();
		Server(ServerManager &server_manager);
		Server(const Server &other);
		Server operator=(const Server &another);
		~Server();
		int		setServer(Config *config); //define server configs
		int		setUpServer(); //define socket and bind to server fd
	
		Config	*getConfig() const { return this->config; } ;
		ServerManager	&getServerManager() const { return this->server_manager; } ;
		int		getServerFd() const { return this->server_fd; } ;
		int		acceptConnection(int epoll_fd);
		int		handleRequest(int fd);
		int		handleResponse(int fd);
		void	freeServer();
};

#endif
