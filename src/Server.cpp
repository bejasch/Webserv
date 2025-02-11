#include "../headers/AllHeaders.hpp"

Server::Server() : server_fd(0), address(), client_requests(), pending_responses(), config(NULL), server_manager(*(new ServerManager())) {
	// std::cout << "Server default constructor called" << std::endl;
}

Server::Server(ServerManager &server_manager) : server_fd(0), address(), client_requests(), pending_responses(), config(NULL), server_manager(server_manager) {
	// std::cout << "Server default constructor called" << std::endl;
}

Server::Server(const Server &other) : server_fd(other.server_fd), address(other.address), client_requests(other.client_requests), pending_responses(other.pending_responses), config(other.config), server_manager(other.server_manager) {
	// std::cout << "Server copy constructor called" << std::endl;
}

Server Server::operator=(const Server &another) {
	if (this == &another)
		return (*this);
	server_fd = another.server_fd;
	address = another.address;
	client_requests = another.client_requests;
	pending_responses = another.pending_responses;
	config = another.config;
	server_manager = another.server_manager;
	return (*this);
}

Server::~Server() {
	// std::cout << "Server destructor called" << std::endl;
}

int Server::setServer(Config *config) {
	this->config = config;
	if (setUpServer() == 1)
		return (1);
	return (0);
}

int Server::setUpServer() {
	// Creating the file descriptor of the program running the server
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0 || server_fd == -1) {
		std::cerr << "Socket creation failed: " << std::strerror(errno) << std::endl;
		return(1);
	}
	// Set SO_REUSEADDR to reuse the port immediately
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
		std::cerr << "Setsockopt failed: " << std::strerror(errno) << std::endl;
		return(1);
	}
	address.sin_family = AF_INET; // address family used previously
	address.sin_addr.s_addr = INADDR_ANY; // this is my IP address
	address.sin_port = htons( config->getPort() ); // the port I would like to expose
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { // bind server file descriptor to socket address
		std::cerr << "Bind failed: " << std::strerror(errno) << std::endl;
		return(1);
	}
	fcntl(server_fd, F_SETFL, O_NONBLOCK);		// Make server_fd non-blocking
	listen(server_fd, 10);			// 10 defines how many pending connections can be queued before connections are refused.
	return (0);
}

// A new socket is created for the communication with the client
int	Server::acceptConnection(int epoll_fd) {	
	//int addrlen = sizeof(address);
	//client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
	int	client_fd = accept(server_fd, NULL, NULL);
	if (client_fd < 0) {
		std::cerr << "Failed to accept connection: " << std::strerror(errno) << std::endl;
		return (1);
	}
	// Make the new socket non-blocking
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	// Add new_socket (=client_fd) to epoll instance to monitor read events
	epoll_event ev;
	ev.events = EPOLLIN;		// Event for reading data
	ev.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
		std::cerr << "Failed to add client_fd to epoll: " << std::strerror(errno) << std::endl;
		close(client_fd); // Clean up if adding to epoll fails. Might want to use EAGAIN or EINTR
		return (1);
	}
	// Add the new client to the client_requests map
	client_requests.insert(std::make_pair(client_fd, HttpReq()));
	server_manager.clientfd_to_serverfd[client_fd] = this;	
	std::cout << GREEN << "New client connected with client_fd " << client_fd << ", linked to server_fd: " << server_fd << RESET<< std::endl;
	return (0);
}

// Data is available to read from the socket
int	Server::handleRequest(int client_fd) {
	HttpReq	&request = client_requests[client_fd];
	char buffer[30000] = {0};
	int valread = read(client_fd, buffer, sizeof(buffer));
	if (valread <= 0) {
		if (valread == 0)
			std::cerr << "Connection closed by client.\n";
		else
			std::cerr << "Error reading from socket.\n";
		close(client_fd);
		client_requests.erase(client_fd); // Clean up state
		return(1);
	}
	std::cout << "Received " << valread << " bytes from client_fd: " << client_fd << std::endl;
	// Process the incoming data if the request is complete
	if (request.processData(*this, std::string(buffer, valread))) {
		pending_responses[client_fd].handleRequest(request, *this);
		request.print();
		client_requests.erase(client_fd);

		epoll_event ev;
		ev.events = EPOLLOUT;		// replace event with client_fd for writing data back as response
		ev.data.fd = client_fd;
		if (epoll_ctl(server_manager.getEpollFd(), EPOLL_CTL_MOD, client_fd, &ev) == -1) {
			std::cerr << "Failed to add client_fd to epoll for writing: " << std::strerror(errno) << std::endl;
			pending_responses.erase(client_fd);
			close(client_fd); // Clean up if adding to epoll fails. Might want to use EAGAIN or EINTR
			return(1);
		}
	}
	return(0);
}

// When the server is ready to send a response to the client, it calls handleResponse
int		Server::handleResponse(int client_fd) {
	if (pending_responses.find(client_fd) != pending_responses.end()) {
		HttpRes &response = pending_responses[client_fd];
		std::string response_str = response.getResponse();
		const char* response_cstr = response_str.c_str();
		size_t		size = response.getResponseSize();
		if (response_cstr == NULL || size == 0) {
			std::cerr << "Error: Response is empty.\n";
		} else {
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
		}
		pending_responses.erase(client_fd);
	}
	// Remove client_fd from epoll instance
	if (epoll_ctl(server_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL) == -1) {
		std::cerr << "Failed to remove client_fd from epoll: " << std::strerror(errno) << std::endl;
		return(1);
	}
	close(client_fd);  // Close the connection
	return (0);
}

void	Server::freeServer() {
	close(server_fd);
	client_requests.clear();
	pending_responses.clear();
	if (config)
		delete config;
}
