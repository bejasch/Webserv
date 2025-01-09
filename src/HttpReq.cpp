
#include "../headers/AllHeaders.hpp"

// HttpReq::HttpReq() {}
// HttpReq::~HttpReq() {}

int	HttpReq::parse(const std::string &buffer) {
	_buffer_section = 0;
	int http_status = 0;
	try
	{
		if ((http_status = parseStartLine(buffer)) != 200)
			return http_status;
		if ((http_status = parseHeaders(buffer)) != 200)
			return http_status;
		// SpecialHeaders ???
		if ((http_status = parseBody(buffer)) != 200)
			return http_status;
		
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
	_buffer_section = pos + 2;

	std::string	start_line = buffer.substr(0, pos);
	if ((pos = start_line.find(" ")) == std::string::npos || pos == 0)
		return 400;
	_method = start_line.substr(0, pos);
	size_t	prev_pos = pos + 1;
	if ((pos = start_line.find(" ", prev_pos)) == std::string::npos || pos == prev_pos)
		return 400;
	_target = start_line.substr(prev_pos, pos - prev_pos);
	prev_pos = pos + 1;
	if ((pos = start_line.find(" ", prev_pos)) != std::string::npos || pos == prev_pos)
		return 400;
	_protocol = start_line.substr(prev_pos);
	
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

// normally return 505 if invalid protocol
bool	HttpReq::isValidProtocol(std::string &protocol) const {
	if (protocol.length() != 8 || protocol.substr(0, 7) != "HTTP/1.")
		return false;
	if (protocol[7] != '0' && protocol[7] != '1')
		return false;
	return true;
}

void	HttpReq::print() const {
    std::cout << "Method: " << _method << "\n";
    std::cout << "Target: " << _target << "\n";
    std::cout << "Protocol: " << _protocol << "\n";
    std::cout << "Headers:\n";
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        std::cout << "\t" << it->first << ": " << it->second << "\n";
    }
    std::cout << "Body: " << _body << "\n";
}

std::string	HttpReq::getMethod() const { return (_method); }

std::string	HttpReq::getTarget() const { return (_target); }

std::string	HttpReq::getProtocol() const { return (_protocol); }

std::string	HttpReq::getHeader(std::string key) const { return (_headers.at(key)); }

size_t		HttpReq::getBodySize() const { return (_bodySize); }

std::string	HttpReq::getBody() const { return (_body); }

int	HttpReq::parseHeaders(const std::string &buffer) {
	size_t		pos = 0;

	while ((pos = buffer.find("\r\n", _buffer_section)) != std::string::npos) {
		if (pos == _buffer_section)
			break;
		std::string	line = buffer.substr(_buffer_section, pos - _buffer_section);
		size_t		pos_colon = line.find(":");
		if (pos_colon == std::string::npos || pos_colon == 0)
			return 400;
		
		std::string	key = line.substr(0, pos_colon);
		if (line[pos_colon + 1] == ' ')
			pos_colon++; 
		if (line.length() == pos_colon)
			return 400;
		std::string	value = line.substr(pos_colon + 1);
		_headers[key] = value;
		_buffer_section = pos + 2;
	}
	return (200);
}

int	HttpReq::parseBody(const std::string &buffer) {
	if (buffer.find("\r\n", _buffer_section) == _buffer_section) {
		_body = buffer.substr(_buffer_section + 2);
		_bodySize = _body.length();
		return (200);
	}
	return (0);
}
