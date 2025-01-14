#include "../headers/AllHeaders.hpp"

ServerManager::ServerManager() {
    std::cout << "ServerManager default constructor called" << std::endl;
}

ServerManager::~ServerManager() {
}

int ServerManager::setServers(const std::string& config_file)
{
    // this should be a loop that reads the config file and creates a server for each server block
    //for testing assume only one server block
    Server server(*this);
    std::cout << "Setting up server" << std::endl;
    server.setServer(config_file);
    servers.push_back(server);
    return 0;
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
        Server &server = servers[i]; // Access the Server object by index
        int server_fd = server.getServerFd();
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
        Server& server = servers[i];
        // this implies its a new connection, that needs to be added to epoll instance
        if (event.data.fd == server.getServerFd()) {
            //server.handleEvent(event.data.fd, event.events, epoll_fd);
            server.acceptConnection(epoll_fd);
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
