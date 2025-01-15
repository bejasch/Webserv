#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "AllHeaders.hpp"

class ServerManager {
private:
    int epoll_fd;
    std::vector<Server*> servers;
    const static int MAX_EVENTS = 100; // For all servers

public:
    ServerManager();
    ~ServerManager();
    int setServers(const std::string& config_file);
    void startServers(); // Runs the central event loop
    int getEpollFd() const { return epoll_fd; }
    std::map<int, Server*> clientfd_to_serverfd;

private:
    void handleEvents();
    void dispatchEvent(const epoll_event& event);
    int fillConfig(std::string line, std::ifstream &file, Config *config);
    int fillRoute(std::string line, std::ifstream &file, Config *config, Route *route);
};


#endif
