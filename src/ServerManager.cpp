#include "../headers/AllHeaders.hpp"

ServerManager::ServerManager() {
    std::cout << "ServerManager default constructor called" << std::endl;
}

ServerManager::~ServerManager() {
    std::cout << "ServerManager destructor called" << std::endl;
}

int ServerManager::setServers(const std::string &config_file)
{
    std::string line;
    std::ifstream file(config_file); // Open the config file
    Server *server;
    Config *config;
    Route *route;

    if (config_file.find(".conf") == std::string::npos) {
        perror("Invalid configuration file format.");
        return 1; // Return 1 if file is not a .conf file
    }
    if (!file.is_open()) {
        perror("Failed to open configuration file");
        return 1; // Return 1 if file opening fails
    }
    while (std::getline(file, line)) {
        if (line.find("server") != std::string::npos) {
            // Dynamically allocate memory for Server and Config
            server = new Server(*this);
            config = new Config();
            // Call fillConfig to parse and fill the server's configuration
            line = fillConfig(line, file, config);  // Pass the file by reference
        }
        if (line.find("location") != std::string::npos) {
            route = new Route();
            route->setPath(line.substr(line.find("location") + std::string("location").length() + 1, line.find("{") - line.find(" ") - 2));
            fillRoute(line, file, config, route);
        }
    }
    //config->printConfig();  // Print the configuration
    server->setServer(config);  // Set server configuration
    servers.push_back(server);  // Add to server collection
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
    // Add all server_fd to epoll instance to monitor incoming connections
    for (int i = 0; i < servers.size(); ++i) {
        Server *server = servers[i]; // Access the Server object by index
        int server_fd = server->getServerFd();
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = server_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
            perror("Failed to add server_fd to epoll");
            return;
    }
    handleEvents();
    }
}

// Central event loop that distributes events to the appropriate server
void ServerManager::handleEvents() {
    epoll_event events[MAX_EVENTS];

    while (true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n == -1) {
            perror("epoll_wait failed");
            return;
        }
        for (int i = 0; i < n; ++i) {
            dispatchEvent(events[i]);
        }
    }
}

void ServerManager::dispatchEvent(const epoll_event& event) {
    for (int i = 0; i < servers.size(); ++i) {
        Server *server = servers[i];
        // this implies its a new connection, that needs to be added to epoll instance
        if (event.data.fd == server->getServerFd()) {
            //server.handleEvent(event.data.fd, event.events, epoll_fd);
            server->acceptConnection(epoll_fd);
            return;
        }
    }
    for (std::map<int, Server*>::iterator it = clientfd_to_serverfd.begin(); it != clientfd_to_serverfd.end(); ++it) {
        if (event.data.fd == it->first) {
            std::cout << "Existing connection" << std::endl;
            Server *server = it->second;
            server->handleRequest(event.data.fd);
            return;
        }
    }
    //if it gets here, it means the fd is not recognized
    std::cerr << "Unknown fd: " << event.data.fd << std::endl;
}

//TODO: this assumes that there is only one space between the key and value
//TODO: this also assumes that there are tabs in front of the key
std::string ServerManager::fillConfig(std::string line, std::ifstream &file, Config *config) {
    // std::string line;
    while (std::getline(file, line)) {
        if (line.find("{") != std::string::npos)
            return(line); // Stop if we find the closing brace
        // Parse each line and assign values to the config object
        if (line.find("listen") != std::string::npos)
            config->setPort(stringToInt(line.substr(line.find("listen") + std::string("listen").length() + 1, line.find(";") - line.find(" ") - 1)));
        else if (line.find("server_name") != std::string::npos)
            config->setName(line.substr(line.find("server_name") + std::string("server_name").length() + 1, line.find(";") - line.find(" ") - 1));
        else if (line.find("root") != std::string::npos)
            config->setRootDir(line.substr(line.find("root") + std::string("root").length() + 1, line.find(";") - line.find(" ") - 1));
        else if (line.find("client_max_body_size") != std::string::npos)
            config->setMaxBodySize(stringToInt(line.substr(line.find("client_max_body_size") + std::string("client_max_body_size").length() + 1, line.find(";") - line.find(" ") - 1)));
        else if (line.find("index") != std::string::npos)
            config->setDefaultFile(line.substr(line.find("index") + std::string("index").length() + 1, line.find(";") - line.find(" ") - 1));
        else if (line.find("error_page") != std::string::npos) {
            config->setErrorStatus(stringToInt(line.substr(line.find("error_page") + std::string("error_page").length() + 1, line.find(" ") - line.find("error_page") - 1)));
            config->setErrorFile(line.substr(line.find("error_page") + std::string("error_page").length() + 5, line.find(";") - line.find(" ") - 5));
        }
    }
    return ""; // Return 0 to indicate successful parsing
}

int ServerManager::fillRoute(std::string line, std::ifstream &file, Config *config, Route *route) {
    while (std::getline(file, line)) {
        if (line.find("}") != std::string::npos) {
            config->addRoute(route);
            break; // Stop if we find the closing brace
        }
        if (line.find("allow_methods") != std::string::npos) {
            // Parse the allowed methods and set them to the route
            std::string methods = line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1);
            std::vector<std::string> allowed_methods;
            std::string method;
            std::istringstream iss(methods);
            while (std::getline(iss, method, ' ')) {
                allowed_methods.push_back(method);
            }
            route->setAllowedMethods(allowed_methods);
        }
        else if (line.find("alias") != std::string::npos)
            route->setRootDir(line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1));
        else if (line.find("return") != std::string::npos) {
            route->setRedirectStatus(stringToInt(line.substr(line.find("return") + std::string("return").length() + 1, line.find(" ") - line.find("return") - 1)));
            route->setRedirectUrl(line.substr(line.find("return") + std::string("return").length() + 5, line.find(";") - line.find(" ") - 5));
        }
        else if (line.find("index") != std::string::npos && isStandaloneWord(line, "index", line.find("index")))
            route->setIndexFile(line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1));
        else if (line.find("autoindex") != std::string::npos)
            route->setAutoindex(line.substr(line.find(" ") + 1, line.find(";") - line.find(" ") - 1));
    }
    return 0;
}