#include "../headers/AllHeaders.hpp"

HttpReq::HttpReq() : _creationTime(time(0)), _server(NULL), _buffer(""),
	_httpStatus(0), _method(""), _target(""), _protocol(""), _headers(),
	_body(""), _startlineParsed(false), _headersParsed(false),
	_isChunked(false), _bodyComplete(false), _currentChunkSize(0) {
	// std::cout << "HttpReq default constructor called" << std::endl;
}

HttpReq::HttpReq(const HttpReq &other) : _creationTime(other._creationTime),
	_server(other._server), _buffer(other._buffer),	_httpStatus(other._httpStatus), 
	_method(other._method), _target(other._target), _protocol(other._protocol), 
	_headers(other._headers), _body(other._body),
	_startlineParsed(other._startlineParsed), _headersParsed(other._headersParsed),
	_isChunked(other._isChunked), _bodyComplete(other._bodyComplete),
	_currentChunkSize(other._currentChunkSize) {
	// std::cout << "HttpReq copy constructor called" << std::endl;
}

HttpReq	HttpReq::operator=(const HttpReq &another)	{
	if (this == &another)
		return (*this);
	_creationTime = another._creationTime;
	_server = another._server;
	_buffer = another._buffer;
	_httpStatus = another._httpStatus;
	_method = another._method;
	_target = another._target;
	_protocol = another._protocol;
	_headers = another._headers;
	_body = another._body;
	_startlineParsed = another._startlineParsed;
	_headersParsed = another._headersParsed;
	_isChunked = another._isChunked;
	_bodyComplete = another._bodyComplete;
	_currentChunkSize = another._currentChunkSize;
	return (*this);
}

HttpReq::~HttpReq()	{
	// std::cout << "HttpReq destructor called" << std::endl;
}

// true means the full request is assembled (incl errors), false means more data is needed
bool HttpReq::processData(Server &server, const std::string &data) {
	if (!_server)
		_server = &server;
	_buffer += data;
	if (!_startlineParsed && !parseStartLine())	// Parse start line if not already done
		return (true);							// Error occurred or short request in start line

	if (!_headersParsed && !parseHeaders())
		return (false);							// Headers are not fully received yet

	if (_isChunked)								// If the request is chunked, process the chunked body
		return (parseChunkedBody());			// Returns true if the full body is assembled, false otherwise
	
	return (parseBody());					// Full request assembled
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
		return (false);
	if (_target.find("..") != std::string::npos)
		return (false);
	if (_target.find("//") != std::string::npos)
		return (false);
	for (size_t i = 1; i < _target.length(); ++i) {
		if (!isalnum(_target[i]) && _target[i] != '/' && _target[i] != '-' && _target[i] != '_' && _target[i] != '.' && _target[i] != '?')
			return (false);
	}
	return (true);
}

// normally return 505 if invalid protocol
bool	HttpReq::isValidProtocol(void) const {
	if (_protocol.length() != 8 || _protocol.substr(0, 7) != "HTTP/1.")
		return (false);
	if (_protocol[7] != '0' && _protocol[7] != '1')
		return (false);
	return (true);
}


bool	HttpReq::parseChunkedBody(void) {
	size_t pos = 0;
	while (!_buffer.empty()) {
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
	return (false);	// Wait for more data
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

bool HttpReq::verifyHeaders() {
	// Reject completely empty headers
	if (_headers.empty()) {
		std::cerr << RED << "Error: Request contains no headers.\n" << RESET;
		_httpStatus = 400; // Bad Request
		return (false);
	}

	// Check Host header (required in HTTP/1.1)
	if (_protocol == "HTTP/1.1" && _headers.find("host") == _headers.end()) {
		std::cerr << RED << "Error: Missing 'Host' header in HTTP/1.1 request.\n" << RESET;
		_httpStatus = 400; // Bad Request
		return (false);
	}

	// Check for conflicting Content-Length and Transfer-Encoding
	bool	hasContentLength = _headers.find("content-length") != _headers.end();
	bool	hasTransferEncoding = _headers.find("transfer-encoding") != _headers.end();

	if (hasContentLength && hasTransferEncoding) {
		std::cerr << RED << "Error: Both 'Content-Length' and 'Transfer-Encoding' are present.\n" << RESET;
		_httpStatus = 400; // Bad Request
		return (false);
	}

	// Validate Transfer-Encoding
	if (hasTransferEncoding) {
		std::string	encoding = _headers["transfer-encoding"];
		std::transform(encoding.begin(), encoding.end(), encoding.begin(), ::tolower);

		if (encoding == "chunked") {
			_isChunked = true;
		} else {
			std::cerr << RED << "Error: Unsupported Transfer-Encoding: " << encoding << "\n" << RESET;
			_httpStatus = 501; // Not Implemented
			return (false);
		}
	}
	return (true);
}

bool	HttpReq::parseBody(void) {
	if (_headers.find("content-length") != _headers.end()) {
		size_t content_length = std::strtoul(_headers["content-length"].c_str(), NULL, 10);
		if (content_length > static_cast<size_t>(_server->getConfig()->getMaxBodySize())) {
			_httpStatus = 413; // Payload Too Large
			return (true);
		}
		_body += _buffer;
		_buffer.clear();
		if (_body.length() < content_length) {
			return (false);		// Wait for full body
		}
	}
	_httpStatus = 200;
	return (true);
}

void	HttpReq::print(void) const {
	std::cout << BG_WHITE << BLUE << "Request received: " << std::ctime(&_creationTime) << RESET;
	std::cout << BLINK<< GREEN << _method << "    " << CYAN << _target << "    " << YELLOW << _protocol << RESET << "\n";
	if (_isChunked)
		std::cout << "This is a chunked transfer!\n";
	std::cout << MAGENTA << "Headers:\n";
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		std::cout << "\t" << it->first << ": " << it->second << "\n";
	}
	if (_body.size() > 100)
		std::cout << BLUE << "Body(head):\n" << _body.substr(0, 100) << "...\n" << RESET;
	else
		std::cout << BLUE << "Body:\n" << _body << "\n" << RESET;
}

Server				*HttpReq::getServer(void) const { return (_server); }

const std::string	&HttpReq::getMethod(void) const { return (_method); }

const std::string	&HttpReq::getTarget(void) const { return (_target); }

const std::string	&HttpReq::getProtocol(void) const { return (_protocol); }

const std::string	&HttpReq::getHeader(const std::string &key) const {
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end())
		return (it->second);
	static const std::string emptyString = "";
	return (emptyString);
}

const std::map<std::string, std::string>	&HttpReq::getHeaders(void) const { return (_headers); }

const std::string	&HttpReq::getBody(void) { return (_body); }

int					HttpReq::getHttpStatus(void) const { return (_httpStatus); }
