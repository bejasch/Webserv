#include "../headers/AllHeaders.hpp"

Server::Server(ServerManager &server_manager) : server_manager(server_manager) {
	std::cout << "Server default constructor called" << std::endl;
}

Server::~Server() {
    std::cout << "Server destructor called" << std::endl;
}

void Server::setServer(Config *config) {
	this->config = config;
	setUpServer();
}

void Server::setUpServer() {
	// Creating the file descriptor of the program running the server
    server_fd = socket(AF_INET, SOCK_STREAM, 0); // TODO: Here was missing something - shielding and setsockopt
	if (server_fd == 0 || server_fd == -1) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	// Set SO_REUSEADDR to reuse the port immediately
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}

    address.sin_family = AF_INET; // address family used previously
    address.sin_addr.s_addr = INADDR_ANY; // this is my IP address
    address.sin_port = htons( config->getPort() ); // the port I would like to expose
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { // bind server file descriptor to socket address
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}
    // Make server_fd non-blocking
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    listen(server_fd, 10); // 10 defines how many pending connections can be queued before connections are refused.
	//std::cout << "Server set up on port " << config->getPort() << " fd: " << server_fd << std::endl;
}

void Server::acceptConnection(int epoll_fd) {
	int client_fd;
	epoll_event ev;
	//int addrlen = sizeof(address);

	// server_fd is the listening socket, thus we need to create a new socket for the communication with the client. This socket is used for communication.
	//client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
	client_fd = accept(server_fd, NULL, NULL);
	if (client_fd < 0) {
		perror("Failed to accept connection");
		return;
	}
	// Make the new socket non-blocking
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	// Add new_socket (=client_fd) to epoll instance to monitor read events
	ev.events = EPOLLIN; // Event for reading data
	ev.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
		perror("Failed to add client_fd to epoll");
		close(client_fd); // Clean up if adding to epoll fails. Might want to use EAGAIN or EINTR
		return;
	}	///adds `new_socket` (=client_fd) to epoll instance and watch it for EPOLLIN
	// Add the new client to the client_requests map
	client_requests.insert(std::make_pair(client_fd, HttpReq()));
	server_manager.clientfd_to_serverfd[client_fd] = this;	

	std::cout << "New client connected: fd " << client_fd << " linked to server_fd: " << server_fd << std::endl;
}

void	Server::handleRequest(int client_fd) {
	HttpReq	&request = client_requests[client_fd];

	std::cout << "Handling request for client_fd: " << client_fd << std::endl;
	// Data is available to read from the socket
	char buffer[30000] = {0};
	int valread = read(client_fd, buffer, sizeof(buffer));
	std::cout << "Received " << valread << " bytes from client_fd: " << client_fd << std::endl;
	// int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);	// Is this better???

	if (valread == 0) {	// Client closed connection
		std::cerr << "Connection closed by client.\n";
		close(client_fd);
		client_requests.erase(client_fd); // Clean up state
		return;
	} else if (valread == -1) {
		std::cerr << "Error reading from socket.\n";
		close(client_fd);
		client_requests.erase(client_fd); // Clean up state
		return;
	}
	//printf("\n### Received data ([part of a] HTTP Request): \n%s\n^^^^^^^^^^^^\n\n", buffer);

	// Process the incoming data
	if (request.processData(std::string(buffer, valread))) {
		// Construct response directly in the map
		pending_responses[client_fd].handleRequest(request, *this);
		request.print();
		client_requests.erase(client_fd);

		epoll_event ev;
        ev.events = EPOLLOUT;
        ev.data.fd = client_fd;
		if (epoll_ctl(server_manager.getEpollFd(), EPOLL_CTL_MOD, client_fd, &ev) == -1) {
			perror("Failed to add client_fd to epoll for writing");
			pending_responses.erase(client_fd);
			close(client_fd); // Clean up if adding to epoll fails. Might want to use EAGAIN or EINTR
			return;
		}
	}
}

void	Server::handleResponse(int client_fd) {
	std::cout << "Entered handleResponse function in server for clientfd: " << client_fd << std::endl;
    if (pending_responses.find(client_fd) != pending_responses.end()) {
        HttpRes &response = pending_responses[client_fd];
        
        std::string response_str = response.getResponse();
		const char* response_cstr = response_str.c_str();
		size_t		size = response.getResponseSize();
		if (response_cstr == NULL || size == 0) {
			std::cerr << "Error: Response is empty.\n";
			return;
		}
		
		size_t	total_sent = 0;
		int		retry_count = 0;

		while (total_sent < size) {
			ssize_t sent = write(client_fd, response_cstr + total_sent, size - total_sent);
			if (sent < 0) {
				retry_count++;

				// If maximum retries reached, log and stop trying
				if (retry_count >= MAX_RETRY_COUNT) {
					std::cerr << "Error: Failed to write to socket after " << MAX_RETRY_COUNT << " retries.\n";
					break;
				}
				std::cerr << "Warning: Write failed, retrying (" << retry_count << "/" << MAX_RETRY_COUNT << ")...\n";
				continue;
			}
			retry_count = 0;
			total_sent += sent;
		}
		if (total_sent < size)
			std::cerr << "Warning: Only " << total_sent << " out of " << size << " bytes were sent.\n";
		else
			std::cout << "Successfully sent " << total_sent << " bytes to client.\n";

		// Remove client_fd from epoll instance
		if (epoll_ctl(server_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL) == -1) {
			perror("Failed to remove client_fd from epoll");
		} else {
			std::cout << "Successfully removed client_fd from epoll\n";
		}
		close(client_fd);  // Close the connection
		pending_responses.erase(client_fd);
	} else {
		close(client_fd);  // Close the connection
		pending_responses.erase(client_fd);
		std::cerr << "Error: No response found for client_fd: " << client_fd << std::endl;
	}
}

void	Server::freeServer() {
	//TODO: is shutdown() allowed?
	shutdown(server_fd, SHUT_RDWR);
	close(server_fd);
	client_requests.clear();
	delete config;
}
