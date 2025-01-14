#include "../headers/AllHeaders.hpp"

HttpRes::HttpRes() {}
HttpRes::~HttpRes() {}

std::map<std::string, std::string> HttpRes::mimeTypes = {
	{"html", "text/html"},
	{"css", "text/css"},
	{"js", "text/javascript"},
	{"jpg", "image/jpeg"},
	{"jpeg", "image/jpeg"},
	{"png", "image/png"},
	{"gif", "image/gif"},
	{"ico", "image/x-icon"}
};

std::map<int, std::string> HttpRes::statusMessages = {
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {408, "Request Timeout"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"}
};

std::map<int, std::string> statusDescription = {
	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{204, "No Content"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{303, "See Other"},
	{304, "Not Modified"},
	{307, "Temporary Redirect"},
	{308, "Permanent Redirect"},
	{400, "The server could not understand the request due to invalid syntax."},
	{401, "Unauthorized"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{408, "Request Timeout"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{503, "Service Unavailable"},
	{504, "Gateway Timeout"}
};

void	HttpRes::generateErrorResponse(int client_fd) {
	
	_body = "<html><head><title>" + std::to_string(_httpStatus) + " "
			+ _statusMessage + "</title></head><body><h1>"
			+ std::to_string(_httpStatus) + " " + _statusMessage + "</h1>";
	if (statusDescription.find(_httpStatus) != statusDescription.end())
		_body += "<p>" + statusDescription[_httpStatus] + "</p>";
	_body += "</body></html>";


	std::ostringstream response;

    // Status line
    response << "HTTP/1.1 " << _httpStatus << " " << _statusMessage << "\r\n";

    // Headers
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << _body.size() << "\r\n";
    response << "Connection: close\r\n";

    // Blank line to separate headers from body
    response << "\r\n";

    // Body
    response << _body;

	// TODO: Exception handling ???
	sendResponse(client_fd, response.str());
}

void	HttpRes::sendResponse(int client_fd, const std::string &response) {
	const char*	response_cstr = response.c_str();
	size_t		size = response.size();
	if (response_cstr == NULL || size == 0)
		return;
	
	size_t	total_sent = 0;
    int		retry_count = 0;

	while (total_sent < size) {
		ssize_t sent = write(client_fd, response_cstr + total_sent, size - total_sent);
		if (sent < 0) {
			retry_count++;

			// If maximum retries reached, log and stop trying
			if (retry_count >= MAX_RETRY_COUNT) {
				fprintf(stderr, "Error: Failed to write to socket after %d retries.\n", MAX_RETRY_COUNT);
				break;
			}

			// Log a generic error and retry
			fprintf(stderr, "Warning: Write failed, retrying (%d/%d)...\n", retry_count, MAX_RETRY_COUNT);
			continue;
		}
		retry_count = 0;
		total_sent += sent;
	}
	if (total_sent < size)
		fprintf(stderr, "Warning: Only %zu out of %zu bytes were sent.\n", total_sent, size);
	else
		printf("Successfully sent %zu bytes to client.\n", total_sent);
}


void	HttpRes::handleRequest(HttpReq &httpRequest) {
	_protocol = httpRequest.getProtocol();
	_httpStatus = httpRequest.getHttpStatus();
	_statusMessage = statusMessages[_httpStatus];
	_target = httpRequest.getTarget();
    if (_httpStatus != 200) {
		return;
	}
	// - Response headers:
    if (httpRequest.getMethod() == "GET") {
        if (_target == "/info.html")
        {
            _contentType = determineContentType(_target);
            _body = parseFile(_target);
            contentLength = _body.length();
        }
		else if (_target == "/image.jpg")
        {
            
            _contentType = determineContentType(_target);
            _body = parseFile(_target);
            contentLength = _body.length();
        }
		else if (_target == "/giphy.gif")
        {
            
            _contentType = determineContentType(_target);
            _body = parseFile(_target);
            contentLength = _body.length();
        }
        else
        {
            _httpStatus = 404;
        }
    }
    else {
        _httpStatus = 404;
    }
}


std::string HttpRes::determineContentType(const std::string &filename) {
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	// Look up the MIME type in the static map
	std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);

	if (it != mimeTypes.end()) {
		return it->second;
	} else {
		return "text/plain";
	}
}

std::string HttpRes::parseFile(const std::string &filename) {
    std::ifstream file(("data/www" + filename).c_str());
    if (!file.is_open()) {
        return parseFile("/error_404.html");
    }
    std::stringstream buffer;
    buffer << file.rdbuf(); // Read entire file
    std::string body = buffer.str();
    file.close();
    return body;
}

void HttpRes::writeResponse(int client_fd) {
    if (_httpStatus != 200)
		return	(generateErrorResponse(client_fd));
	
	// Build the status line
    std::ostringstream	response_stream;
    response_stream << _protocol << " " << _httpStatus << " " << _statusMessage << "\n";

    // Add headers
    response_stream << "Content-Type: " << _contentType << "\n";
    response_stream << "Content-Length: " << contentLength << "\n\n";
    // response_stream << "Connection: keep-alive\n\n"; //without this we still be stuck in the loop (still issues here)

    // Add body
    response_stream << _body;

	sendResponse(client_fd, response_stream.str());
}


/*
// ---------- OK - response

HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
Content-Length: 55743
Connection: keep-alive
Cache-Control: s-maxage=300, public, max-age=0
Content-Language: en-US
Date: Thu, 06 Dec 2018 17:37:18 GMT
ETag: "2e77ad1dc6ab0b53a2996dfd4653c1c3"
Server: meinheld/0.6.1
Strict-Transport-Security: max-age=63072000
X-Content-Type-Options: nosniff
X-Frame-Options: DENY
X-XSS-Protection: 1; mode=block
Vary: Accept-Encoding,Cookie
Age: 7

<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>A simple webpage</title>
</head>
<body>
  <h1>Simple HTML webpage</h1>
  <p>Hello, world!</p>
</body>
</html>



// ---------- MOVE - response

HTTP/1.1 301 Moved Permanently
Server: Apache/2.4.37 (Red Hat)
Content-Type: text/html; charset=utf-8
Date: Thu, 06 Dec 2018 17:33:08 GMT
Location: https://developer.mozilla.org/ (this is the new link to the resource; it is expected that the user-agent will fetch it)
Keep-Alive: timeout=15, max=98
Accept-Ranges: bytes
Via: Moz-Cache-zlb05
Connection: Keep-Alive
Content-Length: 325 (the content contains a default page to display if the user-agent is not able to follow the link)

<!doctype html>… (contains a site-customized page helping the user to find the missing resource)

*/
