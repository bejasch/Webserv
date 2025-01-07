

#pragma once
#include <string>
#include <map>

class HttpReq {
private:
	std::string	method;
	std::string	target;
	std::string	protocol;
	// *GET /en-US/docs/Web/HTTP/Messages HTTP/1.1*
	std::map<std::string, std::string> headers;
    
	std::string body;
	
	// - Request headers:	*additional context to a request or add extra logic* -> case-insensitive string followed by a colon (:) and a value.
	// - Representation headers:	*if message has a body*
	// - Request body:
public:
	int	parse(std::string buffer);

	// --> Get-methods:
	std::string	getMethod();
	std::string	getTarget();
	std::string	getProtocol();

};
