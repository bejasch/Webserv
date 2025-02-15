#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "AllHeaders.hpp"

class ServerManager {
private:
	int						epoll_fd;
	std::vector<Server*>	servers;
	static volatile sig_atomic_t	stop_flag;

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
		Server *server;
		std::string guestbookName;
		std::string output;
	};
	int		setServers(const std::string& config_file);
	void	startServers(); // Runs the central event loop
	int		getEpollFd() const { return epoll_fd; }
	std::map<int, Server*>			clientfd_to_serverfd;	// Maps client_fd to server
	std::map<int, CgiRequestInfo>	cgi_pipes;				// Maps pipe_fd to pipe.infos
	void	printConfigAll();
	int		portCheck(int port);
	int		checkConfig(Config *config);
	int		handleCGIRequest(int pipe_fd);
	int		handleCGIResponse(int pipe_fd);
	void	checkResponseTimeouts(void);
	void	checkCGITimeouts();
	void	writeCGIResponseGET(CgiRequestInfo requestInfo, const std::string &output);
	void	writeCGIResponsePOST(CgiRequestInfo requestInfo, const std::string &output);
	int		freeResources();

private:
	int		handleEvents();
	int		dispatchEvent(const epoll_event& event);
	std::string	fillConfig(std::string line, std::ifstream &file, Config *config);
	std::string	fillRoute(std::string line, std::ifstream &file, Route *route);
	static void	signalHandler(int signum);
	int		create_base_route(Config *config);
	void	validateRoutes();
};

#endif
