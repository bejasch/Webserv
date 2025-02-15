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
	int					_client_fd;

	bool				_wasRedirected;		// If the request was redirected (to avoid endless redirection loops)

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
	void				handleRequestResponse(HttpReq &httpRequest, Server &server, int client_fd);
	std::string			getResponse(void);

	// --> Get/Set-methods:
	const std::string	&getMethod(void) const;
	const std::string	&getTarget(void) const;
	int					getHttpStatus(void) const;
	Route				*getRoute(void) const;
	
	void				setStatus(int status);
	void				setHttpStatus(int status);
	void				setContentType(const std::string &contentType);
	void				setContentLength(size_t length);
	void 				setTarget(const std::string &target);
	void				setBody(const std::string &body);
	
	Server				*getServer(void) const;
};

#endif
