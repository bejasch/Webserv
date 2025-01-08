
#include "HttpReq.hpp"
#include <iostream>

// HttpReq::HttpReq() {}
// HttpReq::~HttpReq() {}

int	HttpReq::parse(const std::string &buffer) {
	buffer_section_ = 0;
	int http_status = 0;
	try
	{
		if ((http_status = parseStartLine(buffer)) != 200)
			return http_status;
		// if ((http_status = parseHeaders(buffer)) != 200)
		// 	return http_status;
		// if ((http_status = parseSpecialHeaders(buffer)) != 200)
		// 	return http_status;
		
		// if ((http_status = parseBody(buffer)) != 200)
		// 	return http_status;
		
	}
	catch (const std::exception &e)
	{
		std::cerr << "Parsing-exception: " << e.what() << std::endl;
		return 500;
	}
	return 200;
}

int	HttpReq::parseStartLine(const std::string &buffer) {
	size_t		pos = 0;

	if ((pos = buffer.find("\r\n")) == std::string::npos || pos == 0)
		return 400;
	buffer_section_ = pos + 2;

	std::string	start_line = buffer.substr(0, pos);
	if ((pos = start_line.find(" ")) == std::string::npos || pos == 0)
		return 400;
	method = start_line.substr(0, pos);
	size_t	prev_pos = pos + 1;
	if ((pos = start_line.find(" ", prev_pos)) == std::string::npos || pos == prev_pos)
		return 400;
	target = start_line.substr(prev_pos, pos - prev_pos);
	prev_pos = pos + 1;
	if ((pos = start_line.find(" ", prev_pos)) != std::string::npos || pos == prev_pos)
		return 400;
	protocol = start_line.substr(prev_pos);
	
	return (200);
}

bool	HttpReq::isValidMethod(std::string &method) const {
	if (method == "GET" || method == "POST" || method == "PUT" || method == "DELETE")
		return (true);
	return (false);
}

bool	HttpReq::isValidTarget(std::string &target) const {
	if (target.empty() || target[0] != '/')
		return false;
	if (target.find("..") != std::string::npos)
		return false;
	if (target.find("//") != std::string::npos)
		return false;
	for (size_t i = 1; i < target.length(); ++i) {
		if (!isalnum(target[i]) && target[i] != '/' && target[i] != '-' && target[i] != '_' && target[i] != '.' && target[i] != '?')
			return false;
	}
	return (true);
}

bool	HttpReq::isValidProtocol(std::string &protocol) const {
	if (protocol.substr(0, 5) != "HTTP/")
		return false;

	std::string version = protocol.substr(5);
	if (version.size() < 3 || version[0] != '1' || version[1] != '.')
		return 505;
	for (size_t i = 2; i < version.size(); i++)
	{
		if (!isdigit(version[i]))
			return 505;
	}

	return 200;
}


void	HttpReq::print() const {
    std::cout << "Method: " << method << "\n";
    std::cout << "Target: " << target << "\n";
    std::cout << "Protocol: " << protocol << "\n";
    std::cout << "Headers:\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << "\n";
    }
    std::cout << "Body: " << body << "\n";
}



std::string	HttpReq::getMethod() const { return (method); }

std::string	HttpReq::getTarget() const { return (target); }

std::string	HttpReq::getProtocol() const { return (protocol); }

int	HttpReq::parseHeaders(std::string &buffer) {
	(void)buffer;
	return 200;
}
