
#include "../headers/AllHeaders.hpp"

// HttpReq::HttpReq() {}
// HttpReq::~HttpReq() {}

// Helper function to trim leading and trailing whitespaces
std::string	trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t");
	if (start == std::string::npos)
		return ("");
	size_t end = str.find_last_not_of(" \t");
	return (str.substr(start, end - start + 1));
}


// int	HttpReq::parse(const std::string &buffer) {
// 	_buffer_section = 0;
// 	int http_status = 0;
// 	try
// 	{
// 		if ((http_status = parseStartLine()) != 200)
// 			return http_status;
// 		if ((http_status = parseHeaders()) != 200)
// 			return http_status;
// 		// SpecialHeaders ???
// 		if ((http_status = parseBody()) != 200)
// 			return http_status;
		
// 	}
// 	catch (const std::exception &e)
// 	{
// 		std::cerr << "Parsing-exception: " << e.what() << std::endl;
// 		return 500;
// 	}
// 	return 200;
// }

int	HttpReq::parseStartLine(void) {
	size_t		pos = 0;

	if ((pos = _buffer.find("\r\n")) == std::string::npos || pos == 0)
		return (400);
	_buffer_section = pos + 2;

	std::string	start_line = _buffer.substr(0, pos);
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

    if (!isValidMethod())
		return (405);
    if (!isValidTarget())
		return (400);
    if (!isValidProtocol())
		return (505);
		
	return (200);
}

bool	HttpReq::isValidMethod(void) const {
	if (_method == "GET" || _method == "POST" || _method == "PUT" || _method == "DELETE")
		return (true);
	return (false);
}

bool	HttpReq::isValidTarget(void) const {
	if (_target.empty() || _target[0] != '/')
		return false;
	if (_target.find("..") != std::string::npos)
		return false;
	if (_target.find("//") != std::string::npos)
		return false;
	for (size_t i = 1; i < _target.length(); ++i) {
		if (!isalnum(_target[i]) && _target[i] != '/' && _target[i] != '-' && _target[i] != '_' && _target[i] != '.' && _target[i] != '?')
			return false;
	}
	return (true);
}

// normally return 505 if invalid protocol
bool	HttpReq::isValidProtocol(void) const {
	if (_protocol.length() != 8 || _protocol.substr(0, 7) != "HTTP/1.")
		return false;
	if (_protocol[7] != '0' && _protocol[7] != '1')
		return false;
	return true;
}

void	HttpReq::print(void) const {
    std::cout << "Method: " << _method << "\n";
    std::cout << "Target: " << _target << "\n";
    std::cout << "Protocol: " << _protocol << "\n";
    std::cout << "Headers:\n";
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        std::cout << "\t" << it->first << ": " << it->second << "\n";
    }
    std::cout << "Body: " << _body << "\n";
}

std::string	HttpReq::getMethod(void) const { return (_method); }

std::string	HttpReq::getTarget(void) const { return (_target); }

std::string	HttpReq::getProtocol(void) const { return (_protocol); }

std::string	HttpReq::getHeader(std::string key) const { return (_headers.at(key)); }

size_t		HttpReq::getBodySize(void) const { return (_bodySize); }

std::string	HttpReq::getBody(void) const { return (_body); }


// TODO: How to react if an error occurs during (previous) parsing ???
bool HttpReq::processData(const std::string &data) {
	_buffer += data;

	if (!_startlineParsed) {		// Parse start line if not already done
		if (!parseStartLine()) {
			return (false);
		}
	}
	if (!_headersParsed) {		// Parse headers if not already done
		if (!parseHeaders()) {
			// Headers are not fully received yet
			return (false);
		}
	}
	// If the request is chunked, process the chunked body
	if (_isChunked)
		return (parseChunkedBody()); // Returns true if the full body is assembled, false otherwise
	parseBody();

	return (true); // Full request assembled
}

bool	HttpReq::parseChunkedBody(void) {
    while (!_buffer.empty()) {
        if (currentChunkSize == 0) {
            // Parse chunk size
            size_t pos = _buffer.find("\r\n");
            if (pos == std::string::npos) {
                return false; // Wait for full chunk size
            }
            std::string sizeStr = _buffer.substr(0, pos);
            std::istringstream sizeStream(sizeStr);
            sizeStream >> std::hex >> currentChunkSize;
            _buffer = _buffer.substr(pos + 2); // Remove chunk size and CRLF

            if (currentChunkSize == 0) {
                // End of chunks
                return true; // Full body assembled
            }
        }

        // Read chunk data
        if (_buffer.size() >= currentChunkSize + 2) { // Include trailing CRLF
            _body += _buffer.substr(0, currentChunkSize);
            _buffer = _buffer.substr(currentChunkSize + 2); // Remove chunk data and CRLF
            currentChunkSize = 0; // Reset for the next chunk
        } else {
            return false; // Wait for more data
        }
    }

    return (false); // Not fully assembled yet
}


bool	HttpReq::parseHeaders(void) {
	size_t		pos = 0;

	// Check if headers are fully received
	if (_buffer.find("\r\n\r\n") == std::string::npos) {
		// Headers are incomplete, wait for more data
		return (false);
	}

	while ((pos = _buffer.find("\r\n", _buffer_section)) != std::string::npos) {
		if (pos == _buffer_section) {
			_buffer_section += 2;
			_headersParsed = true;
			break;
		}
		
		std::string	line = _buffer.substr(_buffer_section, pos - _buffer_section);
		_buffer_section = pos + 2;
		
		if (line.length() > MAX_HEADER_SIZE)
			return (_httpStatus = 413, true); // Payload Too Large
		if (_headers.size() >= MAX_HEADER_COUNT)
			return (_httpStatus = 431, true); // Request Header Fields Too Large
		
		size_t		pos_colon = line.find(":");
		if (pos_colon == std::string::npos || pos_colon == 0 || pos_colon == line.length())
			return (_httpStatus = 400, true);
		
		std::string	key = trim(line.substr(0, pos_colon));
		std::string	value = trim(line.substr(pos_colon + 1));
		
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		if (key.empty() || value.empty() || _headers.find(key) != _headers.end())
			return (_httpStatus = 400, true);

		_headers[key] = value;
	}
	return (true);
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

void	HttpReq::parseBody(void) {
	if (_buffer.find("\r\n", _buffer_section) != _buffer_section) {
		_body = _buffer.substr(_buffer_section + 2);
		_bodySize = _body.length();
		return (_httpStatus = 200, void());
	}
	_httpStatus = 400; // TODO: Check if this is the correct status code
}

// Reset for a new request - or better destroy and create a new object ?
void	HttpReq::reset(void) {
	_headers.clear();
}
