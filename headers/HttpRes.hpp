#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include "AllHeaders.hpp"

class HttpRes {
private:
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

	static std::map<std::string, std::string>	mimeTypes;
	static std::map<int, std::string>			statusDescription;

	void				getNameCookie(HttpReq &httpRequest);
	bool				parseFile(void);
	void				determineContentType(void);

	// - Response methods:
	void				GET(void);
	void				POST(HttpReq &httpRequest);
	void				DELETE(void);

	void				generateAutoindexPage(const std::string &path);
	void				generateErrorBody(void);
	
public:
	HttpRes();
	HttpRes(const HttpRes &other);
	HttpRes operator=(const HttpRes &another);
	~HttpRes();

	void		handleRequest(HttpReq &httpRequest, Server &server);
	std::string	getResponse(void);
	size_t		getResponseSize(void) const;

	// --> Get-methods:
	const std::string	&getMethod(void) const;
	const std::string	&getTarget(void) const;
	Route				*getRoute(void) const;
	void				setStatus(int status) { _httpStatus = status; }


};

#endif
