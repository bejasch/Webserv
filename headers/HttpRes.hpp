
#pragma once
#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include "AllHeaders.hpp"

class HttpRes {
private:
	std::string			_method;
	int					_httpStatus;
	size_t				_responseSize;

	std::string			_target;
	std::string			_contentType;
	std::string			_body;

	static std::map<std::string, std::string>	mimeTypes;
	static std::map<int, std::string>			statusDescription;


	bool				parseFile(Server &server);
	void				determineContentType(void);

	// - Response methods:
	void				GET(HttpReq &httpRequest, Server &server, Route *route);
	void				POST(HttpReq &httpRequest, Server &server);
	void				DELETE(const std::string &path);

	void				generateAutoindexPage(const std::string &path);
	void				generateErrorBody(void);

	void				executeCGI(HttpReq &httpRequest, Server &server);

public:
	HttpRes();
	HttpRes(const HttpRes &other);
	HttpRes operator=(const HttpRes &another);
	~HttpRes();

	void		handleRequest(HttpReq &httpRequest, Server &server);
	std::string	getResponse(void);
	size_t		getResponseSize(void) const;

};

#endif
