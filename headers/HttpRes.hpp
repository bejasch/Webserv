
#pragma once
#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include "AllHeaders.hpp"

class HttpRes {
private:
	// - Response headers:
	std::string	_protocol;
	int			_httpStatus;

	std::string	_target;
	std::string	_statusMessage;
	std::string	_contentType;
	int			contentLength;
	std::string	_body;

	std::string parseFile(const std::string &filename);
	std::string determineContentType(const std::string &filename);

	static std::map<int, std::string>			statusMessages;
	static std::map<std::string, std::string>	mimeTypes;
	static std::map<int, std::string>			statusDescriptions;

	void	sendResponse(int client_fd, const std::string &response);

public:
	HttpRes();
	~HttpRes();

	void		handleRequest(HttpReq &httpRequest);
	void		generateErrorResponse(int client_fd);
	void		writeResponse(int client_fd);

};

#endif
