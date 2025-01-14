

#pragma once
#ifndef HTTPREQ_HPP
#define HTTPREQ_HPP

#include "AllHeaders.hpp"

class HttpReq {
private:
	std::string	_buffer;
	int			_httpStatus;

	std::string	_method;
	std::string	_target;
	std::string	_protocol;

	std::map<std::string, std::string> _headers;	// Headers are case-insensitive (tolower-ed)
    
	size_t		_bodySize;
	std::string _body;

	bool	_startlineParsed = false;	// Whether the start line is fully received
	bool	_headersParsed = false;		// Whether the headers are fully received
	bool	_isChunked = false;     	// Whether the body uses chunked transfer
	bool	_bodyComplete = false;  	// Whether the body is fully received

    size_t	_currentChunkSize = 0;		// Size of the current chunk being read
	
	// Helper function to trim leading and trailing whitespaces
	std::string	trim(const std::string& str);
	bool		verifyHeaders(void);
	bool		headersAreParsed(void) const;


	// - Request headers:	*additional context to a request or add extra logic* -> case-insensitive string followed by a colon (:) and a value.
	// - Representation headers:	*if message has a body*
	// - Request body:
	bool	parseStartLine(void);
	bool	isValidMethod(void) const;
	bool	isValidTarget(void) const;
	bool	isValidProtocol(void) const;


	bool	parseHeaders(void);
	void	parseBody(void);
	bool	parseChunkedBody(void);

public:
	// int	parse(const std::string &buffer);

	// --> Get-methods:
	std::string	getMethod(void) const;
	std::string	getTarget(void) const;
	std::string	getProtocol(void) const;
	std::string	getHeader(std::string key) const;
	size_t		getBodySize(void) const;
	std::string	getBody(void) const;
	int			getHttpStatus(void) const;

	bool 	processData(const std::string &data);	// Encapsulates the parsing of the incoming data

	// print content
	void	print(void) const;

	// Reset for a new request
    void	reset(void);
};

#endif
