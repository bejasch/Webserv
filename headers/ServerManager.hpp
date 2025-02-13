#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "AllHeaders.hpp"

class ServerManager {
private:
	int epoll_fd;
	std::vector<Server*> servers;
	static volatile sig_atomic_t stop_flag;
	std::map<int, std::string> cgi_outputs;

public:
	ServerManager();
	ServerManager(const ServerManager &other);
	ServerManager operator=(const ServerManager &another);
	~ServerManager();
	struct CgiRequestInfo {
		int client_fd;
		std::string method;
		pid_t pid;
		time_t start_time;
	};
	int setServers(const std::string& config_file);
	void startServers(); // Runs the central event loop
	int getEpollFd() const { return epoll_fd; }
	std::map<int, Server*> clientfd_to_serverfd;
	std::map<int, CgiRequestInfo>	cgi_pipes;	// Maps pipe_fd to pipe.infos
	// std::map<int, pid_t>	cgi_pids;
	// std::map<int, time_t>	cgi_start_times;
	void printConfigAll();
	int portCheck(int port);
	int checkConfig(Config *config);
	int	handleCGIResponse(int pipe_fd);
	void	checkCGITimeouts();
	void writeCGIResponseGET(int client_fd, const std::string &output);
	void writeCGIResponsePOST(int client_fd, const std::string &output);
	int freeResources();

private:
	int handleEvents();
	int dispatchEvent(const epoll_event& event);
	std::string fillConfig(std::string line, std::ifstream &file, Config *config);
	std::string fillRoute(std::string line, std::ifstream &file, Route *route);
	static void signalHandler(int signum);
	int create_base_route(Config *config);
	void validateRoutes();
};

#endif
