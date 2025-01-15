#include "../headers/AllHeaders.hpp"

Server::Server(ServerManager &server_manager) : server_manager(server_manager) {
	std::cout << "Server default constructor called" << std::endl;
}

Server::~Server() {
    std::cout << "Server destructor called" << std::endl;
}

void Server::setServer(const std::string& server_config) {
	//server_config should contain the server block
	// Config config;
	//config.parseConfigFile(server_config);
	addServer(Config()); // Add a server with default config
	addRoute(Route()); // Add a route with default values
	setUpServer();
}

void Server::addServer(const Config& config) {
	this->config = config;
}

void Server::addRoute(const Route& route) {
	routes.push_back(route);
}

void Server::setUpServer() {
	// Creating the file descriptor of the program running the server
    server_fd = socket(AF_INET, SOCK_STREAM, 0); //

    address.sin_family = AF_INET; // address family used previously
    address.sin_addr.s_addr = INADDR_ANY; // this is my IP address
    address.sin_port = htons( config.getPort() ); // the port I would like to expose
    bind(server_fd, (struct sockaddr *)&address, sizeof(address)); // bind server file descriptor to socket address

    // Make server_fd non-blocking
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    listen(server_fd, 10); // 10 defines how many pending connections can be queued before connections are refused.
}

void Server::acceptConnection(int epoll_fd) {
	int client_fd;
	epoll_event ev;
	int addrlen = sizeof(address);

	// server_fd is the listening socket, thus we need to create a new socket for the communication with the client. This socket is used for communication.
	client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
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
	client_requests[client_fd] = HttpReq();
	server_manager.clientfd_to_serverfd[client_fd] = this;	

	std::cout << "New client connected: fd " << client_fd << std::endl;
}

void Server::handleRequest(int client_fd) {
	HttpReq	&request = client_requests[client_fd];
	HttpRes httpResponse;

	// Data is available to read from the socket
	char buffer[30000] = {0};
	int valread = read(client_fd, buffer, sizeof(buffer));
	// int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);	// Is this better???

	if (valread == 0) {	// Client closed connection
		printf("Connection closed by client.\n");
		close(client_fd);
		client_requests.erase(client_fd); // Clean up state
		return;
	} else if (valread == -1) {
		printf("Error reading from socket.\n");
		close(client_fd);
		client_requests.erase(client_fd); // Clean up state
		return;
	}
	printf("\n### Received data ([part of a] HTTP Request): \n%s\n^^^^^^^^^^^^\n\n", buffer);

	// Process the incoming data
	if (request.processData(std::string(buffer, valread))) {
		int parse_status = request.getHttpStatus();
		std::cout << "\nDATA READING FINISHED with status: " << parse_status << std::endl;
		client_requests[client_fd].print();
		
		httpResponse.handleRequest(request);
		httpResponse.writeResponse(client_fd);

		// // Full request assembled
		// handleFullRequest(client_fd, request);
		// request.reset(); // Reset for a new request
		client_requests.erase(client_fd);
		close(client_fd);
	}
}

int Server::getServerFd() {
	return this->server_fd;
}
