#pragma once
#ifndef CGI_HPP
#define CGI_HPP

#include "AllHeaders.hpp"

class CGI {
private:
	pid_t                               pid;
	std::map<std::string, std::string>  env;
	char                                **envp;
	char						        **argv;

public:
	CGI();
	~CGI();

	void   		setAllEnv(HttpRes &httpResponse);
	std::string	executeCGI_GET(HttpRes &httpResponse);
	std::string	executeCGI_POST(HttpRes &httpResponse, const std::map<std::string, std::string> &formData);

	void freeEnvironment();
	void printCGI();
};

#endif