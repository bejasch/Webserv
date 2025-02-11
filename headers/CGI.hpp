#ifndef CGI_HPP
#define CGI_HPP

#include "AllHeaders.hpp"

class CGI {
private:
	pid_t								pid;
	std::map<std::string, std::string>  env;
	char								**envp;
	char								**argv;

public:
	CGI();
	CGI(const CGI &other);
	CGI operator=(const CGI &another);
	~CGI();

	int			setAllEnv(HttpRes &httpResponse);
	std::string	executeCGI_GET(HttpRes &httpResponse, int client_fd);
	std::string	executeCGI_POST(HttpRes &httpResponse, const std::map<std::string, std::string> &formData);

	void freeEnvironment();
	void printCGI();
};

#endif
