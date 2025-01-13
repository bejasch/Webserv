
#include "../headers/AllHeaders.hpp"

// HttpReq::HttpReq() {}
// HttpReq::~HttpReq() {}

// Helper function to trim leading and trailing whitespaces
std::string	HttpReq::trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t");
	if (start == std::string::npos)
		return ("");
	size_t end = str.find_last_not_of(" \t");
	return (str.substr(start, end - start + 1));
}

// false means errors occurred, true means the startline does not contain errors
bool	HttpReq::parseStartLine(void) {
	size_t		pos = 0;

	if ((pos = _buffer.find("\r\n")) == std::string::npos || pos == 0)
		return (_httpStatus = 400, false);

	std::string	start_line = _buffer.substr(0, pos);
	_buffer = _buffer.substr(pos + 2);
	if ((pos = start_line.find(" ")) == std::string::npos || pos == 0)
		return (_httpStatus = 400, false);
	_method = start_line.substr(0, pos);
	size_t	prev_pos = pos + 1;
	if ((pos = start_line.find(" ", prev_pos)) == std::string::npos || pos == prev_pos)
		return (_httpStatus = 400, false);
	_target = start_line.substr(prev_pos, pos - prev_pos);
	prev_pos = pos + 1;
	if ((pos = start_line.find(" ", prev_pos)) != std::string::npos || pos == prev_pos)
		return (_httpStatus = 400, false);
	_protocol = start_line.substr(prev_pos);

    if (!isValidMethod())
		return (_httpStatus = 405, false);
    if (!isValidTarget())
		return (_httpStatus = 400, false);
    if (!isValidProtocol())
		return (_httpStatus = 505, false);
		
	_startlineParsed = true;
	return (_httpStatus = 200, true);
}

bool	HttpReq::isValidMethod(void) const {
	return (_method == "GET" || _method == "POST" || _method == "DELETE");
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
	std::cout << "Chunked transfer: " << _isChunked << "\n";
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

int			HttpReq::getHttpStatus(void) const { return (_httpStatus); }

// TODO: How to react if an error occurs during (previous) parsing ???
// true means the full request is assembled (incl errors), false means more data is needed
bool HttpReq::processData(const std::string &data) {
	_buffer += data;
	if (!_startlineParsed && !parseStartLine())	// Parse start line if not already done
		return (true);

	if (!_headersParsed && !parseHeaders())
		return (false);							// Headers are not fully received yet
	// std::cout << "HERE!" << std::endl;
	if (_isChunked)						// If the request is chunked, process the chunked body
		return (parseChunkedBody());	// Returns true if the full body is assembled, false otherwise
	// std::cout << "DATA NOT CHUNKED!" << std::endl;
	return (parseBody(), true); // Full request assembled
}

bool	HttpReq::parseChunkedBody(void) {
	size_t pos = 0;
	std::cout << "### Parsing chunked body..." << std::endl;
	while (!_buffer.empty()) {
		// std::cout << "Current buffer: " << _buffer << std::endl;
		if (_currentChunkSize == 0) {	// Parse new chunk size -> hex number followed by CRLF
			if ((pos = _buffer.find("\r\n")) == std::string::npos) {
				return (false); // Wait for full chunk size
			} else if (pos == 0)
				return (_httpStatus = 400, true); // Invalid chunk size

			std::string sizeStr = _buffer.substr(0, pos);
			std::istringstream sizeStream(sizeStr);
			sizeStream >> std::hex >> _currentChunkSize;
			if (sizeStream.fail() || !sizeStream.eof()) {
				_httpStatus = 400;
				return (true); // Invalid chunk size
			}
			// std::cout << "Current chunk size: " << _currentChunkSize << std::endl;
			_buffer = _buffer.substr(pos + 2); // Remove chunk size and CRLF

			if (_currentChunkSize == 0) {	// End of chunks
				if (_buffer.size() > 2) {
					_httpStatus = 400;
					return (true); // Invalid chunks
				}
				return (_bodyComplete = true, true); // Full body assembled
			}
		}
		// Read chunk data
		if (_buffer.size() >= _currentChunkSize + 2) { // Include trailing CRLF
			if (_buffer[_currentChunkSize] != '\r' || _buffer[_currentChunkSize + 1] != '\n')
				return (_httpStatus = 400, true); // Invalid chunk data
			_body += _buffer.substr(0, _currentChunkSize);
			_buffer = _buffer.substr(_currentChunkSize + 2); // Remove chunk data and CRLF
			_currentChunkSize = 0;	// Reset for the next chunk
		} else
			break;
	}
	std::cout << "### Not fully assembled yet..." << std::endl;
	return (false); // Not fully assembled yet
}

// Returns true if headers are fully received (or on error), false otherwise
bool	HttpReq::parseHeaders(void) {
	size_t		pos;

	// Check if headers are fully received
	if ((pos = _buffer.find("\r\n\r\n")) == std::string::npos)	// Headers are incomplete, wait for more data
		return (false);
	else if (pos == 0) {	// Empty line -> end of headers
		_buffer = _buffer.substr(4);
		_headersParsed = true;
		return (true);
	}

	while ((pos = _buffer.find("\r\n")) != std::string::npos) {
		if (pos == 0) {
			_buffer = _buffer.substr(2);
			_headersParsed = true;
			break;
		}
		
		std::string	line = _buffer.substr(0, pos);
		_buffer = _buffer.substr(pos + 2);
		
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
	verifyHeaders();
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

bool	HttpReq::verifyHeaders() {
	// Check required headers
	if (_headers.find("host") == _headers.end()) {
		std::cerr << "Error: Missing 'Host' header.\n";
		return (false);
	}

	if (_headers.find("content-length") != _headers.end() &&
		_headers.find("transfer-encoding") != _headers.end()) {
		std::cerr << "Error: Both 'Content-Length' and 'Transfer-Encoding' are present.\n";
		return (false);
	}

	// Check if Transfer-Encoding is chunked
	std::map<std::string, std::string>::iterator it = _headers.find("transfer-encoding");
	if (it != _headers.end()) {
		std::string encoding = it->second;
		std::transform(encoding.begin(), encoding.end(), encoding.begin(), ::tolower);
		if (encoding == "chunked") {
			_isChunked = true;
		} else {
			std::cerr << "Error: Unsupported Transfer-Encoding: " << encoding << "\n";
			return (false);
		}
	}
	return (true);
}

void	HttpReq::parseBody(void) {
	size_t		pos;
	if (_headers.find("content-length") != _headers.end()) {
		size_t	content_length = std::stoul(_headers["content-length"]);
		if (_buffer.length() < content_length)	{
			_httpStatus = 400;
			return;
		}
		_body = _buffer.substr(0, content_length);
		_bodySize = content_length;
		return (_httpStatus = 200, void());
	}
	_httpStatus = 400; // TODO: Check if this is the correct status code
}

// Reset for a new request - or better destroy and create a new object ?
void	HttpReq::reset(void) {
	_headers.clear();
}
