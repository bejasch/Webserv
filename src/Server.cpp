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
	std::cout << GREEN << "\nNew client connected with client_fd " << client_fd << ", linked to server_fd: " << server_fd << RESET<< std::endl;
	return (0);
}

// Data is available to read from the socket
int	Server::handleRequestServer(int client_fd) {
	HttpReq	&request = client_requests[client_fd];
	char buffer[30000] = {0};
	int valread = read(client_fd, buffer, sizeof(buffer));
	if (valread <= 0) {
		close(client_fd);
		client_requests.erase(client_fd); // Clean up state
		if (valread == 0) {
			std::cerr << RED << "Connection closed by client with client_fd " << client_fd << RESET << std::endl;
			return(0);
		}
		std::cout << "client_fd: " << client_fd << std::endl;
		std::cerr << "Error reading from socket.\n";
		return(1);
	}
	std::cout << BOLD << "\n\tReceived " << valread << " bytes from client_fd: " << client_fd << std::endl << RESET;
	// Process the incoming data if the request is complete
	if (request.processData(*this, std::string(buffer, valread))) {
		pending_responses[client_fd].handleRequestResponse(request, *this, client_fd);
		request.print();
		client_requests.erase(client_fd);
		if (pending_responses[client_fd].getHttpStatus() == 0) {
			pending_responses.erase(client_fd);
			if (epoll_ctl(server_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL) == -1)
				std::cerr << "Failed to add client_fd to epoll for writing: " << std::strerror(errno) << std::endl;
			return (0);
		}

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

// EPOLLOUT was triggered on an existing connection (event)
int		Server::handleResponse(int client_fd) {
	if (pending_responses.find(client_fd) != pending_responses.end()) {

		std::string &response_str = pending_responses[client_fd].getResponse();
		size_t		size = response_str.size();
		const char* response_cstr = response_str.c_str();
		if (response_cstr == NULL || size == 0) {
			std::cerr << "Response was empty -> Deleting pending response\n";
		} else {
			ssize_t sent = write(client_fd, response_cstr, size);
			if (sent < 0) {
				std::cerr << "Writing to socket failed.\n";
			}
			else if (sent < static_cast<ssize_t>(size)) {	// Not all data was sent -> client remains in epoll for writing
				response_str = response_str.substr(sent);
				std::cout << "Warning: Only " << sent << " out of " << size << " bytes were sent to client_fd: " << client_fd << std::endl;
				return (0);
			}
			else
				std::cout << BOLD << "\tSuccessfully sent " << sent << " bytes to client_fd: " << client_fd << std::endl << RESET;
		}
		pending_responses.erase(client_fd);
	}
	// Remove client_fd from epoll instance
	if (epoll_ctl(server_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL) == -1) {
		std::cerr << "Failed to remove client_fd from epoll: " << std::strerror(errno) << std::endl;
		return(1);
	}
	close(client_fd);  // Close the connection
	std::cout << "Close the connection to the client_fd: " << client_fd << std::endl;
	return (0);
}

void	Server::freeServer() {
	close(server_fd);
	client_requests.clear();
	pending_responses.clear();
	if (config)
		delete config;
}

ServerManager					&Server::getServerManager() const {
	return this->server_manager;
}

std::map<int, HttpRes>	&Server::getPendingResponses() {
	return this->pending_responses;
}


void	Server::deleteClientResponse(int client_fd) {
	pending_responses.erase(client_fd);
	std::cout << "deleted client fd :" << client_fd << " from pending responses" << std::endl;
}

void	Server::addPendingResponse(int client_fd, HttpRes &response) {
	if (pending_responses.find(client_fd) != pending_responses.end())
		pending_responses.erase(client_fd);
	pending_responses[client_fd] = response;
}

void	Server::deletePendingResponse(int client_fd) {
	pending_responses.erase(client_fd);
}
