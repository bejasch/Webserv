

#pragma once
#ifndef HTTPREQ_HPP
#define HTTPREQ_HPP

#include "AllHeaders.hpp"

class HttpReq {
private:
	size_t	_buffer_section;

	// - Request headers:
	std::string	_method;
	std::string	_target;
	std::string	_protocol;
	// *GET /en-US/docs/Web/HTTP/Messages HTTP/1.1*
	std::map<std::string, std::string> _headers;
    
	size_t		_bodySize;
	std::string _body;
	
	// Helper function to trim leading and trailing whitespaces
	std::string	trim(const std::string& str);
	bool	HttpReq::verifyHeaders() const;

	// - Request headers:	*additional context to a request or add extra logic* -> case-insensitive string followed by a colon (:) and a value.
	// - Representation headers:	*if message has a body*
	// - Request body:
	int		parseStartLine(const std::string &buffer);
	bool	isValidMethod(std::string &method) const;
	bool	isValidTarget(std::string &target) const;
	bool	isValidProtocol(std::string &protocol) const;

	int		parseHeaders(const std::string &buffer);
	int		parseBody(const std::string &buffer);

public:
	int	parse(const std::string &buffer);

	// --> Get-methods:
	std::string	getMethod() const;
	std::string	getTarget() const;
	std::string	getProtocol() const;
	std::string	getHeader(std::string key) const;
	size_t		getBodySize() const;
	std::string	getBody() const;

	// print content
	void	print() const;

};

#endif
