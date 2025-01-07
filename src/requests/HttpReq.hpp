

#pragma once
#include <string>
#include <map>

class HttpReq {
private:

	// - Request line:	*method, target, protocol*
	std::string line;

	// - Request headers:
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
	int	parse(std::string &buffer);

	int	parseStartLine(std::string line);
	int	parseHeaders(std::string line);
	int	parseBody(std::string line);

	// --> Get-methods:
	std::string	getMethod();
	std::string	getTarget();
	std::string	getProtocol();

};


HttpReq::HttpReq() {}
HttpReq::~HttpReq() {}
int	HttpReq::parse(std::string &buffer) {
	std::string line;
	std::string::size_type pos = 0;
	std::string::size_type prev = 0;
	while ((pos = buffer.find("\r\n", prev)) != std::string::npos) {
		line = buffer.substr(prev, pos - prev);
		if (line.empty()) {
			break;
		}
		if (prev == 0) {
			parseStartLine(line);
		} else {
			parseHeaders(line);
		}
		prev = pos + 2;
	}
	parseBody(buffer.substr(prev));
	return 0;
}
