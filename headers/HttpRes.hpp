
#pragma once
#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include "AllHeaders.hpp"

class HttpRes {
private:
	// - Response headers:
	std::string	_protocol;
	int			_httpStatus;

	std::string status_message;
	std::string	content_type;
	int	content_length;
	std::string	body;

	std::string parseFile(const std::string &filename);
	std::string determineContentType(const std::string &filename);

	static std::map<int, std::string> statusMessages;
	static std::map<std::string, std::string> mimeTypes;

public:
	HttpRes();
	~HttpRes();
	void handleRequest(HttpReq &httpRequest);
	void writeResponse(int client_fd);

};

#endif
