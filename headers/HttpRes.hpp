
#pragma once
#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include "AllHeaders.hpp"

class HttpRes {
private:
	std::string			_method;
	int					_httpStatus;

	std::string			_target;
	std::string			_contentType;
	std::string			_body;
	std::map<std::string, std::string>	_headers;

	static std::map<int, std::string>			statusMessages;
	static std::map<std::string, std::string>	mimeTypes;
	static std::map<int, std::string>			statusDescriptions;


	bool				parseFile(Server &server);
	void				determineContentType(void);

	// - Response methods:
	void				GET(HttpReq &httpRequest, Server &server, Route *route);
	void				POST(HttpReq &httpRequest, Server &server);
	void				DELETE(const std::string &path);

	// std::map<std::string, std::string>	parsePostData(const std::string& data);
	void				generateAutoindexPage(const std::string &path);
	void				generateErrorBody(void);

	void	sendResponse(int client_fd, const std::string &response);

	void	executeCGI(HttpReq &httpRequest, Server &server);

public:
	HttpRes();
	~HttpRes();

	void		handleRequest(HttpReq &httpRequest, Server &server);
	void		writeResponse(int client_fd);

};

#endif
