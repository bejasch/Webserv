

#pragma once
#include <string>
#include <map>

class HttpReq {
private:
	size_t	buffer_section_;

	// - Request headers:
	std::string	method;
	std::string	target;
	std::string	protocol;
	// *GET /en-US/docs/Web/HTTP/Messages HTTP/1.1*
	std::map<std::string, std::string> headers;
    
	size_t		bodySize;
	std::string body;
	
	// - Request headers:	*additional context to a request or add extra logic* -> case-insensitive string followed by a colon (:) and a value.
	// - Representation headers:	*if message has a body*
	// - Request body:
	int		parseStartLine(const std::string &buffer);
	bool	isValidMethod(std::string &method) const;
	bool	isValidTarget(std::string &target) const;
	bool	isValidProtocol(std::string &protocol) const;

	int		parseHeaders(std::string &buffer);

public:
	int	parse(const std::string &buffer);

	// int	parseSpecialHeaders(std::string &buffer);

	// int	parseBody(std::string &buffer);
	
	// --> Get-methods:
	std::string	getMethod() const;
	std::string	getTarget() const;
	std::string	getProtocol() const;

	// print content
	void	print() const;

};
