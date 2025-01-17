
#pragma once
#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include "AllHeaders.hpp"

class HttpRes {
private:
	// - Response headers:
	std::string	_protocol;
	std::string	_method;
	int			_httpStatus;

	std::string	_target;
	std::string	_statusMessage;
	std::string	_contentType;
	int			contentLength;
	std::string	_body;

	std::string parseFile(const std::string &filename, Server &server);
	std::string determineContentType(const std::string &filename);

	static std::map<int, std::string>			statusMessages;
	static std::map<std::string, std::string>	mimeTypes;
	static std::map<int, std::string>			statusDescriptions;

	const std::string	GUESTBOOK_FILE = "guestbook.txt";

	std::map<std::string, std::string>	parsePostData(const std::string& data);

	std::string	generateGuestbookHTML(void);
	void		saveEntry(const std::string& name, const std::string& message);
	
	void	generateErrorResponse(int client_fd);

	void	sendResponse(int client_fd, const std::string &response);

public:
	HttpRes();
	~HttpRes();

	void		handleRequest(HttpReq &httpRequest, Server &server);
	void		writeResponse(int client_fd);

};

#endif
