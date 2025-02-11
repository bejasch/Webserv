#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include "AllHeaders.hpp"

class HttpRes {
private:
	// - Static variables:
	Server				*_server;
	Route				*_route;

	std::string			_method;
	int					_httpStatus;
	size_t				_responseSize;
	std::string			_userName;

	std::string			_target;
	std::string			_serverPath;
	std::string			_contentType;
	std::string			_body;

	bool				_wasRedirected;		// If the request was redirected (to avoid double free)

	void				getNameCookie(HttpReq &httpRequest);	// Get the name of the user from the cookie
	bool				parseFile(void);						
	void				determineContentType(void);				

	const std::string	&getMimeType(const std::string &extension);
	const std::string	&getStatusDescription(int status);

	// - Response methods:
	void				GET(void);
	void				POST(HttpReq &httpRequest);
	void				DELETE(void);

	void				generateAutoindexPage(const std::string &path);
	void				generateErrorBody(void);
	
public:
	HttpRes(void);
	HttpRes(const HttpRes &other);
	HttpRes operator=(const HttpRes &another);
	~HttpRes();

	// --> Response methods:
	void				handleRequest(HttpReq &httpRequest, Server &server);
	std::string			getResponse(void);
	size_t				getResponseSize(void) const;

	// --> Get/Set-methods:
	const std::string	&getMethod(void) const;
	const std::string	&getTarget(void) const;
	Route				*getRoute(void) const;
	void				setStatus(int status);
	Server				*getServer(void) const;
};

#endif
