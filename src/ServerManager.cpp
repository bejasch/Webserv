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
        perror("Invalid configuration file format.");
        return (1); // TODO: remember to free (no need to free here, goes out of scope)
    }
    if (!file.is_open()) {
        perror("Failed to open configuration file");
        return (1); // TODO: remember to free (no need to free here, goes out of scope)
    }
    while (std::getline(file, line)) {
        if (line.find("server") != std::string::npos) {
            // Dynamically allocate memory for Server and Config
            server = new Server(*this);
            config = new Config();
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
            route->setPath(line.substr(line.find("location") + std::string("location").length() + 1, line.find("{") - line.find(" ") - 2));
            line = fillRoute(line, file, config, route);
            if (line.empty()) {
                std::cerr << "Discarding unfinished route: " << route->getPath() << std::endl;
                delete route;
                route = NULL;
                continue; // Skip to the next line in the config
            }
            if (route->checkRoute(route) == 1) {
                delete route;
                route = NULL; // Avoid dangling pointer
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
    printConfigAll();  // Print the configuration
    file.close();  // Close the file explicitly (optional since it's auto-closed on scope exit)
    return 0;  // Return 0 to indicate success
}

void ServerManager::startServers() {
    // // Create epoll instance
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Failed to create epoll instance");
        return;
    }
    if (signal(SIGINT, ServerManager::signalHandler) == SIG_ERR) {
        std::perror("signal");
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
            perror("Failed to add server_fd to epoll");
            return;
            }
        //std::cout << "Added server_fd: " << server_fd << " to epoll" << std::endl;
    }
    handleEvents();
}

// Central event loop that distributes events to the appropriate server
void ServerManager::handleEvents() {
    epoll_event events[MAX_EVENTS];

    while (!stop_flag) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n == -1) {
            perror("epoll_wait failed");
            return;
        }
        for (int i = 0; i < n; ++i) {
            dispatchEvent(events[i]);
        }
    }
    freeResources();
}

void ServerManager::dispatchEvent(const epoll_event& event) {
    for (unsigned long i = 0; i < servers.size(); ++i) {
        Server *server = servers[i];
        // this implies its a new connection, that needs to be added to epoll instance
        if (event.data.fd == server->getServerFd()) {
            server->acceptConnection(epoll_fd);
            return;
        }
    }
    for (std::map<int, Server*>::iterator it = clientfd_to_serverfd.begin(); it != clientfd_to_serverfd.end(); ++it) {
        if (event.data.fd == it->first) {
            Server *server = it->second;
			if (event.events & EPOLLIN) {
            	server->handleRequest(event.data.fd);	// Handle request for client_fd
			} else if (event.events & EPOLLOUT) {
				server->handleResponse(event.data.fd);	// Handle response for client_fd
			}
            return;
        }
    }
    //if it gets here, it means the fd is not recognized
    std::cerr << "Unknown fd: " << event.data.fd << std::endl;
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

//TODO: this assumes that there is only one space between the key and value
//TODO: this also assumes that there are tabs in front of the key
std::string ServerManager::fillConfig(std::string line, std::ifstream &file, Config *config) {
    // std::string line;
    while (std::getline(file, line)) {
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
            config->setDefaultFile(line.substr(line.find("index") + std::string("index").length() + 1, line.find(";") - line.find(" ") - 1)); //TODO: currently only supports one index file
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
    return NULL; // Return 0 to indicate successful parsing
}

std::string ServerManager::fillRoute(std::string line, std::ifstream &file, Config *config, Route *route) {
	(void)config;
    int braceCount = 1;  // Track number of open braces
    while (std::getline(file, line)) {
        if (line.find("{") != std::string::npos)
            braceCount++;
        if (line.find("}") != std::string::npos) {
            braceCount--;
            if (braceCount == 0)
                return line;
        }
        if (braceCount < 0) {
            std::cerr << "Error: Mismatched braces in config file!" << std::endl;
            return ""; // Indicate an error
        }
        if (line.find("location") != std::string::npos && braceCount == 2)
        {
            route->setPath(line.substr(line.find("location") + std::string("location").length() + 1, line.find("{") - line.find(" ") - 2));
            route->cleanRoute(route);
        }
        if (line.find("allow_methods") != std::string::npos) {
            // Parse the allowed methods and set them to the route
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
            if (line.find("on") != std::string::npos)
                route->setAutoindex(true);
            else if (line.find("off") != std::string::npos)
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
            // TODO: cgi routes are also getting filled with the base route's values
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

