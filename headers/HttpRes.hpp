#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include "AllHeaders.hpp"

class HttpRes {
private:
	// - Static variables:
	int					_client_fd;
	Server				*_server;
	Route				*_route;

	std::string			_method;
	int					_httpStatus;
	std::string			_userName;

	std::string			_target;
	std::string			_serverPath;
	std::string			_contentType;
	std::string			_body;

	bool				_wasRedirected;		// If the request was redirected (to avoid endless redirection loops)

	bool				_responseCreated;	// If the full response was created
	std::string			_response;			// The response to be sent to the client or left part that was not sent yet

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

	void				createFullResponse(void);
	
public:
	HttpRes(void);
	HttpRes(const HttpRes &other);
	HttpRes operator=(const HttpRes &another);
	~HttpRes();

	// --> Response methods:
	void				handleRequestResponse(HttpReq &httpRequest, Server &server, int client_fd);
	std::string			&getResponse(void);

	// --> Get/Set-methods:
	const std::string	&getMethod(void) const;
	const std::string	&getTarget(void) const;
	int					getHttpStatus(void) const;
	Route				*getRoute(void) const;
	
	void				setStatus(int status);
	void				setHttpStatus(int status);
	void				setContentType(const std::string &contentType);
	void 				setTarget(const std::string &target);
	void				setBody(const std::string &body);
	
	Server				*getServer(void) const;
};

#endif
