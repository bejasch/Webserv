#ifndef HTTPREQ_HPP
#define HTTPREQ_HPP

#include "AllHeaders.hpp"

class HttpReq {
private:
	// - Static variables:
	time_t		_creationTime;
	Server		*_server;

	std::string	_buffer;
	int			_httpStatus;

	std::string	_method;
	std::string	_target;
	std::string	_protocol;

	std::map<std::string, std::string> _headers;	// Headers are case-insensitive (tolower-ed)
	
	std::string _body;

	bool		_startlineParsed;	// Whether the start line is fully received
	bool		_headersParsed;		// Whether the headers are fully received
	bool		_isChunked;     	// Whether the body uses chunked transfer
	bool		_bodyComplete;  	// Whether the body is fully received
	size_t		_currentChunkSize;	// Size of the current chunk being read
	
	// Helper function to trim leading and trailing whitespaces
	bool	verifyHeaders(void);
	bool	headersAreParsed(void) const;

	bool	parseStartLine(void);
	bool	isValidMethod(void) const;
	bool	isValidTarget(void) const;
	bool	isValidProtocol(void) const;


	bool	parseHeaders(void);
	bool	parseBody(void);
	bool	parseChunkedBody(void);

public:
	HttpReq();
	HttpReq(const HttpReq &other);
	HttpReq operator=(const HttpReq &another);
	~HttpReq();

	// --> Get-methods:
	const std::string	&getMethod(void) const;
	const std::string	&getTarget(void) const;
	const std::string	&getProtocol(void) const;
	const std::string	&getHeader(const std::string &key) const;
	const std::map<std::string, std::string>	&getHeaders(void) const;
	
	// --> Processing methods:
	bool	processData(Server &server, const std::string &data);	// Encapsulates the parsing of the incoming data
	const std::string	&getBody(void);								// Get the body of the request
	int					getHttpStatus(void) const;					// Get the status of the request

	void	print(void) const;										// print content

	void	reset(void);											// Reset for a new request
};

#endif
