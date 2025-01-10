
#include "../headers/AllHeaders.hpp"

// HttpReq::HttpReq() {}
// HttpReq::~HttpReq() {}

// Helper function to trim leading and trailing whitespaces
std::string	trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t");
	size_t end = str.find_last_not_of(" \t");
	return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}


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
		return (400);
	_buffer_section = pos + 2;

	std::string	start_line = buffer.substr(0, pos);
	if ((pos = start_line.find(" ")) == std::string::npos || pos == 0)
		return (400);
	_method = start_line.substr(0, pos);
	size_t	prev_pos = pos + 1;
	if ((pos = start_line.find(" ", prev_pos)) == std::string::npos || pos == prev_pos)
		return (400);
	_target = start_line.substr(prev_pos, pos - prev_pos);
	prev_pos = pos + 1;
	if ((pos = start_line.find(" ", prev_pos)) != std::string::npos || pos == prev_pos)
		return (400);
	_protocol = start_line.substr(prev_pos);

    if (!isValidMethod(_method))
		return (405);
    if (!isValidTarget(_target))
		return (400);
    if (!isValidProtocol(_protocol))
		return (505);
		
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
		_buffer_section = pos + 2;
		
		if (line.length() > MAX_HEADER_SIZE)
			return 413; // Payload Too Large
		if (_headers.size() >= MAX_HEADER_COUNT)
			return 431; // Request Header Fields Too Large
		
		size_t		pos_colon = line.find(":");
		if (pos_colon == std::string::npos || pos_colon == 0 || pos_colon == line.length())
			return 400;
		
		std::string	key = trim(line.substr(0, pos_colon));
		std::string	value = trim(line.substr(pos_colon + 1));
		
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		
		if (key.empty() || value.empty() || _headers.find(key) != _headers.end())
			return 400;

		_headers[key] = value;
	}
	return (200);
}

// bool	HttpReq::verifyHeaders() const {
// 	if (_headers.find("host") == _headers.end())
// 		return (false);
// 	if (_headers.find("content-length") != _headers.end()) {
// 		if (_headers["content-length"].find_first_not_of("0123456789") != std::string::npos)
// 			return (false);
// 		if (std::stoul(_headers["content-length"]) != _bodySize)
// 			return (false);
// 	}
// 	return (true);
// }

int	HttpReq::parseBody(const std::string &buffer) {
	if (buffer.find("\r\n", _buffer_section) == _buffer_section) {
		_body = buffer.substr(_buffer_section + 2);
		_bodySize = _body.length();
		return (200);
	}
	return (0);
}
