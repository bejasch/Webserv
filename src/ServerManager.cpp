#include "../headers/AllHeaders.hpp"

volatile sig_atomic_t ServerManager::stop_flag = 0;

ServerManager::ServerManager(): epoll_fd(0), servers(), clientfd_to_serverfd() {
	// std::cout << "ServerManager default constructor called" << std::endl;
}

ServerManager::ServerManager(const ServerManager &other): epoll_fd(other.epoll_fd), servers(other.servers), clientfd_to_serverfd(other.clientfd_to_serverfd) {
	// std::cout << "ServerManager copy constructor called" << std::endl;
}

ServerManager ServerManager::operator=(const ServerManager &another) {
	if (this == &another)
		return (*this);
	epoll_fd = another.epoll_fd;
	servers = another.servers;
	clientfd_to_serverfd = another.clientfd_to_serverfd;
	return (*this);
}

ServerManager::~ServerManager() {
	freeResources();
	// std::cout << "ServerManager destructor called" << std::endl;
}

int ServerManager::setServers(const std::string &config_file)
{
	std::string line;
	std::ifstream file(config_file.c_str());	// Open the config file
	Server *server = NULL;
	Config *config = NULL;
	Route *route = NULL;

	if (config_file.find(".conf") == std::string::npos) {
		std::cerr << "Invalid configuration file format: " << std::strerror(errno) << std::endl;
		return (1);
	}
	if (!file.is_open()) {
		std::cerr << "Failed to open configuration file: " << std::strerror(errno) << std::endl;
		return (1);
	}
	while (std::getline(file, line)) {
		if (line.empty() || find_commented_line(line) == 1) {
			continue; // Skip empty lines and comments
		}
		if (line.find("server") != std::string::npos) {
			// Dynamically allocate memory for Server and Config
			server = new Server(*this);
			config = new Config();
			if (server == NULL || config == NULL) {
				std::cerr << "Failed to allocate memory for server or config: " << std::strerror(errno) << std::endl;
				return 1;
			}
			// Call fillConfig to parse and fill the server's configuration
			line = fillConfig(line, file, config);  // Pass the file by reference
			if (checkConfig(config) == 1) {
				delete server;
				delete config;
				server = NULL;
				config = NULL;
				continue;
			}
			server->setServer(config);
			std::string host_key = config->getName() + ":" + intToString(config->getPort());
			std::cout << GREEN << "💻Server added with host_name: " << host_key << RESET <<std::endl;
			create_base_route(config);
			servers.push_back(server);
		}
		if (line.find("location") != std::string::npos && server != NULL) {
			route = new Route();
			if (route == NULL) {
				std::cerr << "Failed to allocate memory for route: " << std::strerror(errno) << std::endl;
				return 1;
			}
			std::string path = line.substr(line.find("location") + std::string("location").length() + 1, line.find("{") - line.find(" ") - 2);
			route->setPath(removeTrailingSlash(path));
            std::cout << BLUE << "\t🌱 Route path: " << route->getPath() << RESET << std::endl;
			line = fillRoute(line, file, route);
			if (line.empty()) {
				std::cerr << "Discarding unfinished route: " << route->getPath() << std::endl;
				delete route;
				route = NULL;
				continue;
			}
			if (route->checkRoute(route) == 1) {
				delete route;
				route = NULL;
				continue;
			}
			config->addRoute(route);
		}
	}
	if (servers.size() == 0) {
		std::cerr << "No correctly initialised servers found in configuration file" << std::endl;
		return 1;
	}
	validateRoutes();
	//printConfigAll();
	file.close();
	return 0;
}

void ServerManager::startServers() {
	// // Create epoll instance
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		std::cerr << "Failed to create epoll instance: " << std::strerror(errno) << std::endl;
		return;
	}
	if (signal(SIGINT, ServerManager::signalHandler) == SIG_ERR) {
		std::cerr << "Error with signal: " << std::strerror(errno) << std::endl;
		std::exit(EXIT_FAILURE);
	}
	// Add all server_fd to epoll instance to monitor incoming connections
	for (unsigned long i = 0; i < servers.size(); ++i) {
		Server *server = servers[i]; // Access the Server object by index
		int server_fd = server->getServerFd();
		epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = server_fd;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
			std::cerr << "Failed to add server_fd to epoll: " << std::strerror(errno) << std::endl;
			return;
		}
	}
    std::cout << GREEN << "🚀Servers are running!" << RESET << std::endl;
	handleEvents();
}

// check CGI timeouts
void	ServerManager::checkCGITimeouts() {
	std::cout << "Checking CGI timeouts" << std::endl;
    time_t now = time(NULL);
    std::map<int, CgiRequestInfo>::iterator it = cgi_pipes.begin();
    while (it != cgi_pipes.end()) {
        int pipe_fd = it->first;
        time_t start_time = it->second.start_time;
		std::cout << "Time difference: " << now - start_time << std::endl;
        if (now - start_time > CGI_TIMEOUT) {
            pid_t pid = it->second.pid;
            std::cout << "Timeout exceeded → Kill CGI process " << pid << std::endl;
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);

            // Cleanup
            if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_fd, NULL) == -1) {
				std::cerr << "Failed to remove pipe from epoll: " << std::strerror(errno) << std::endl;
			}
            close(pipe_fd);
            cgi_pipes.erase(pipe_fd);
            it = cgi_pipes.begin();
        } else {
            ++it; // Move to next element if no timeout
        }
    }
}

// Central event loop that distributes events to the appropriate server
int ServerManager::handleEvents() {
	epoll_event events[MAX_EVENTS];

	while (!stop_flag) {
		int n = epoll_wait(epoll_fd, events, MAX_EVENTS, WAIT_CHECK);
		if (n == -1) {
			if (errno == EINTR) continue; // Ignore interrupted syscalls
			std::cerr << "Epoll_wait failed: " << std::strerror(errno) << std::endl;
			return (1);
		}
		for (int i = 0; i < n; ++i) {
			if (dispatchEvent(events[i])) {
				std::cerr << "Error handling event" << std::endl;
			}
		}
    	checkCGITimeouts();
	}
	freeResources();
	return (0);
}

int ServerManager::dispatchEvent(const epoll_event& event) {
	if (cgi_pipes.find(event.data.fd) != cgi_pipes.end()) {
		std::cout << "CGI response received on pipe_fd: " << event.data.fd << std::endl;
		return (handleCGIResponse(event.data.fd));
	}
	for (unsigned long i = 0; i < servers.size(); ++i) {
		Server *server = servers[i];
		// this implies its a new connection, that needs to be added to epoll instance
		if (event.data.fd == server->getServerFd()) {
			if (server->acceptConnection(epoll_fd)) {
				std::cerr << "Failed to accept connection for server_fd: " << server->getServerFd() << std::endl;
				return (1);
			}
			return (0);
		}
	}
	for (std::map<int, Server*>::iterator it = clientfd_to_serverfd.begin(); it != clientfd_to_serverfd.end(); ++it) {
		if (event.data.fd == it->first) {
			Server *server = it->second;
			if (event.events & EPOLLIN) {
				if (server->handleRequest(event.data.fd))	// Handle request for client_fd
					return (1);
			} else if (event.events & EPOLLOUT) {
				if (server->handleResponse(event.data.fd))	// Handle response for client_fd
					return (1);
			}
			return (0);
		}
	}
	//if it gets here, it means the fd is not recognized
	std::cerr << "Unknown fd: " << event.data.fd << std::endl;
	return (1);
}

int ServerManager::create_base_route(Config *config) {
	Route *route = new Route();
	route->setPath("/");
	route->setAllowedMethods(config->getAllowedMethods());
	route->setRootDirRoute(config->getRootDirConfig());
	route->setIndexFile(config->getDefaultFile());
	config->addRoute(route);
	return 0;
}

std::string ServerManager::fillConfig(std::string line, std::ifstream &file, Config *config) {
	while (std::getline(file, line)) {
		if (line.empty() || find_commented_line(line) == 1) {
			continue;
		}
		if (line.find("{") != std::string::npos || line.find("}") != std::string::npos)
			return(line); // Stop if we find the closing brace
		if (line.find("listen") != std::string::npos)
			config->setPort(stringToInt(line.substr(line.find("listen") + std::string("listen").length() + 1, line.find(";") - line.find(" ") - 1)));
		else if (line.find("server_name") != std::string::npos)
			config->setName(line.substr(line.find("server_name") + std::string("server_name").length() + 1, line.find(";") - line.find(" ") - 1));
		else if (line.find("root") != std::string::npos)
		{
			std::string root_dir = line.substr(line.find("root") + std::string("root").length() + 1, line.find(";") - line.find(" ") - 1);
			config->setRootDirConfig(removeTrailingSlash(root_dir));
		}
		else if (line.find("client_max_body_size") != std::string::npos)
			config->setMaxBodySize(stringToInt(line.substr(line.find("client_max_body_size") + std::string("client_max_body_size").length() + 1, line.find(";") - line.find(" ") - 1)));
		else if (line.find("index") != std::string::npos)
		{
			std::string index_file = line.substr(line.find("index") + std::string("index").length() + 1, line.find(";") - line.find(" ") - 1);
			config->setDefaultFile(removeTrailingSlash(index_file));
		}
		else if (line.find("error_page") != std::string::npos) {
			int error_status = stringToInt(line.substr(line.find("error_page") + std::string("error_page").length() + 1, line.find(" ") - line.find("error_page") - 1));
			std::string error_page = line.substr(line.find("error_page") + std::string("error_page").length() + 5, line.find(";") - line.find(" ") - 5);
			config->setErrorPage(error_status, removeTrailingSlash(error_page));
		}
		else if (line.find("allow_methods") != std::string::npos) {
			std::string methods = line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1);
			config->setAllowedMethods(splitString(methods, ' '));
		}
		else if (!line.empty())
			std::cerr << "Unknown directive in Server Configs: " << line << std::endl;
	}
	return NULL;
}

std::string ServerManager::fillRoute(std::string line, std::ifstream &file, Route *route) {
	int braceCount = 1;
	while (std::getline(file, line)) {
		if (line.empty() || find_commented_line(line) == 1) {
			continue;
		}
		if (line.find("}") != std::string::npos) {
			braceCount--;
			if (braceCount == 0)
				return line;
		}
		if (braceCount < 0) {
			std::cerr << "Error: Mismatched braces in config file!" << std::endl;
			return NULL;
		}
		if (line.find("location") != std::string::npos)
		{
			route->setPath(line.substr(line.find("location") + std::string("location").length() + 1, line.find("{") - line.find(" ") - 2));
			route->cleanRoute(route);
		}
		else if (line.find("allow_methods") != std::string::npos) {
			std::string methods = line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1);
			route->setAllowedMethods(splitString(methods, ' '));
		}
		else if (line.find("root") != std::string::npos)
		{
			std::string root = line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1);
			route->setRootDirRoute(removeTrailingSlash(root));
		}
		else if (line.find("return") != std::string::npos) {
			route->setRedirectStatus(stringToInt(line.substr(line.find("return") + std::string("return").length() + 1, line.find(" ") - line.find("return") - 1)));
			std::string redirect_url = line.substr(line.find("return") + std::string("return").length() + 5, line.find(";") - line.find(" ") - 5);
			route->setRedirectUrl(removeTrailingSlash(redirect_url));
		}
		else if (line.find("index") != std::string::npos && isStandaloneWord(line, "index", line.find("index")))
		{
			std::string index = line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1);
			route->setIndexFile(removeTrailingSlash(index));
		}
		else if (line.find("autoindex") != std::string::npos)
		{
			std::string autoindex = line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1);
			if (autoindex.find("on") != std::string::npos)
				route->setAutoindex(true);
			else if (autoindex.find("off") != std::string::npos)
				route->setAutoindex(false);
			else
				std::cerr << "Invalid autoindex directive: " << line << std::endl;
		}
		else if (!line.empty())
			std::cerr << "Unknown directive: " << line << std::endl;
	}
	std::cerr << "Error: Unclosed route block detected." << std::endl;
	return "";
}

void ServerManager::printConfigAll() {
	for (unsigned long i = 0; i < servers.size(); i++) {
		servers[i]->getConfig()->printConfig();
	}
}

int ServerManager::portCheck(int port) {
	for (unsigned long i = 0; i < servers.size(); i++) {
		if (servers[i]->getConfig()->getPort() == port) {
			std::cerr << "Server not added. Port " << port << " already in use." << std::endl;
			return 1;
		}
	}
	return 0;
}

int ServerManager::checkConfig(Config *config) {
	if (portCheck(config->getPort()) == 1) {
		return 1;
	}
	if (config->initialisedCheck() == 1) {
		return 1;
	}
	return 0;
}

void ServerManager::signalHandler(int signum) {
	if (signum == SIGINT) {
		stop_flag = 1;
	}
}

static bool	compareRoutes(Route* a, Route* b) {
	return (a->getPath().size() < b->getPath().size());
}

void ServerManager::validateRoutes() {
	for (size_t i = 0; i < servers.size(); i++) {
		Config *config = servers[i]->getConfig();
		std::vector<Route *> routes = config->getRoutes();

		// Sort routes by path length (shorter paths first)
		std::sort(routes.begin(), routes.end(), compareRoutes);

		Route *baseRoute = NULL;
		for (size_t j = 0; j < routes.size(); j++) {
			Route *route = routes[j];
			if (route->getPath() == "/"){
				baseRoute = route;
				continue;
			}

			// Find parent route
			Route *parent = baseRoute;
			for (size_t k = 0; k < j; k++) {  // Only check previously processed routes
				// Split paths into segments to compare directories
				std::string routePath = route->getPath();
				std::string parentPath = routes[k]->getPath();

				// Compare the directory structure
				if (routePath.substr(0, parentPath.size()) == parentPath && 
					(routePath[parentPath.size()] == '/' || routePath.size() == parentPath.size())) {
					parent = routes[k];
				}
			}

			// If a parent is found, inherit missing config values
			if (parent) {
				if (route->getRootDirRoute().empty()) {
					route->setRootDirRoute(parent->getRootDirRoute());
				}
				if (route->getIndexFile().empty()) {
					route->setIndexFile(parent->getIndexFile());
				}
				if (route->getAllowedMethods().empty()) {
					route->setAllowedMethods(parent->getAllowedMethods());
				}
				if (route->getRedirectStatus() == 0) {
					route->setRedirectStatus(parent->getRedirectStatus());
				}
				if (route->getRedirectUrl().empty()) {
					route->setRedirectUrl(parent->getRedirectUrl());
				}
				if (!route->getAutoindexSet()) {
					route->setAutoindex(parent->getAutoindex());
				}
			}
		}
	}
}

int ServerManager::handleCGIResponse(int pipe_fd) {
    std::cout << "Handling CGI response" << std::endl;
    char buffer[30000] = {0};
    int bytes_read;
    
    // Read CGI output
    while (true) {
        bytes_read = read(pipe_fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            cgi_outputs[pipe_fd].append(buffer, bytes_read);
            std::cout << "Read " << bytes_read << " bytes from CGI output\n";
        }
        else if (bytes_read == 0) {
            std::cout << "CGI process closed the pipe.\n";
            break;
        }
        else if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return 0;  // Keep the pipe in epoll and wait for more data
            }
            std::cerr << "Read error: " << strerror(errno) << std::endl;
            break;
        }
    }

    // Find the client_fd that requested the CGI execution
	CgiRequestInfo requestInfo = cgi_pipes[pipe_fd];
    int client_fd = requestInfo.client_fd;
    std::string method = requestInfo.method;
    std::string output = cgi_outputs[pipe_fd];

	std::cout << "client fd" << client_fd << std::endl;

    // Clean up pipe resources
    close(pipe_fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_fd, NULL);
    cgi_pipes.erase(pipe_fd);
    cgi_outputs.erase(pipe_fd);

    // If no output was received, return error
    if (output.empty()) {
        std::cerr << "Error: CGI script produced no output.\n";
        close(client_fd);
        return 1;
    }

	std::cout << "output: " << output << std::endl;

	// Send the CGI output to the client
	if (method == "GET")
		writeCGIResponseGET(client_fd, output);
	else if (method == "POST")
		writeCGIResponsePOST(client_fd, output);

    std::cout << "CGI response sent to client_fd: " << client_fd << std::endl;
    
    // Clean up client connection
	// Server *server = clientfd_to_serverfd[client_fd];
	// server->deleteClientResponse(client_fd);
	// Remove client_fd from epoll instance
	close(client_fd);  // Close the connection
	std::cout << "client fd closed: " << client_fd << std::endl;
    return 0;
}

void ServerManager::writeCGIResponseGET(int client_fd, const std::string &output) {
	std::string response_str;
	HttpRes httpResponse;
	httpResponse.setHttpStatus(200);
	httpResponse.setContentType("text/html");
	httpResponse.setBody(output);
	response_str = httpResponse.getResponse();
	const char* response_cstr = response_str.c_str();
	size_t		size = httpResponse.getResponseSize();
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
			std::cout << BOLD << "\tSuccessfully sent " << total_sent << " bytes to client.\n" << RESET;
	}
}

void ServerManager::writeCGIResponsePOST(int client_fd, const std::string &output) {
	(void)client_fd;
	saveGuestbookEntry("Default", output);

}

int ServerManager::freeResources() {
	if (!servers.empty()) {
		for (unsigned long i = 0; i < servers.size(); i++) {
			servers[i]->getConfig()->freeConfig();
			servers[i]->freeServer();
			delete servers[i];
		}
		servers.clear();
	}
	else
		return 0;
	// Close epoll file descriptor
	if (epoll_fd != -1) {
		close(epoll_fd);
		epoll_fd = -1;
	}
	// Clear the client-to-server mapping
	clientfd_to_serverfd.clear();
	cgi_pipes.clear();
	return 0;
}
