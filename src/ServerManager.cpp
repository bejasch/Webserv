#include "../headers/AllHeaders.hpp"

volatile sig_atomic_t ServerManager::stop_flag = 0;

ServerManager::ServerManager() {
    std::cout << "ServerManager default constructor called" << std::endl;
}

ServerManager::~ServerManager() {
    std::cout << "ServerManager destructor called" << std::endl;
    freeResources();
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
            std::cout << "Server added with host_name: " << host_key << std::endl;
            create_base_route(config);
            servers.push_back(server);
        }
        if (line.find("location") != std::string::npos && server != NULL) {
            route = new Route();
            if (route == NULL) {
                std::cerr << "Failed to allocate memory for route: " << std::strerror(errno) << std::endl;
                return 1;
            }
            route->setPath(line.substr(line.find("location") + std::string("location").length() + 1, line.find("{") - line.find(" ") - 2));
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
    // printConfigAll();
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
    handleEvents();
}

// Central event loop that distributes events to the appropriate server
int ServerManager::handleEvents() {
    epoll_event events[MAX_EVENTS];

    while (!stop_flag) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n == -1) {
            std::cerr << "Epoll_wait failed: " << std::strerror(errno) << std::endl;
            return (1);
        }
        for (int i = 0; i < n; ++i) {
            if (dispatchEvent(events[i])) {
                std::cerr << "Error handling event" << std::endl;
                //freeResources();
                return (1);
            }
        }
    }
    freeResources();
    return (0);
}

int ServerManager::dispatchEvent(const epoll_event& event) {
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
            config->setRootDirConfig(line.substr(line.find("root") + std::string("root").length() + 1, line.find(";") - line.find(" ") - 1));
        else if (line.find("client_max_body_size") != std::string::npos)
            config->setMaxBodySize(stringToInt(line.substr(line.find("client_max_body_size") + std::string("client_max_body_size").length() + 1, line.find(";") - line.find(" ") - 1)));
        else if (line.find("index") != std::string::npos)
            config->setDefaultFile(line.substr(line.find("index") + std::string("index").length() + 1, line.find(";") - line.find(" ") - 1));
        else if (line.find("error_page") != std::string::npos) {
            int error_status = stringToInt(line.substr(line.find("error_page") + std::string("error_page").length() + 1, line.find(" ") - line.find("error_page") - 1));
            std::string error_page = line.substr(line.find("error_page") + std::string("error_page").length() + 5, line.find(";") - line.find(" ") - 5);
            config->setErrorPage(error_status, error_page);
        }
        else if (line.find("allow_methods") != std::string::npos) {
            // Parse the allowed methods and set them to the route
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
            route->setRootDirRoute(line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1));
        else if (line.find("return") != std::string::npos) {
            route->setRedirectStatus(stringToInt(line.substr(line.find("return") + std::string("return").length() + 1, line.find(" ") - line.find("return") - 1)));
            route->setRedirectUrl(line.substr(line.find("return") + std::string("return").length() + 5, line.find(";") - line.find(" ") - 5));
        }
        else if (line.find("index") != std::string::npos && isStandaloneWord(line, "index", line.find("index")))
            route->setIndexFile(line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1));
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
            std::cerr << "Port " << port << " already in use" << std::endl;
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


