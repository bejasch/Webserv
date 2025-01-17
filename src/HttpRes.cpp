#include "../headers/AllHeaders.hpp"

HttpRes::HttpRes() {
    std::cout << "HttpRes default constructor called" << std::endl;
}
HttpRes::~HttpRes() {
    std::cout << "HttpRes destructor called" << std::endl;
}

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

// Parse HTTP POST data (application/x-www-form-urlencoded)
// Example body: "name=Ben&message=This+is+a+test%21"
std::map<std::string, std::string> HttpRes::parsePostData(const std::string& data) {
    std::map<std::string, std::string> postData;
    std::istringstream stream(data);
    std::string pair;

    while (std::getline(stream, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            // Decode URL-encoded values (basic implementation)
            std::replace(value.begin(), value.end(), '+', ' ');
			// Decode URL-encoded values (extended implementation)
			for (size_t i = 0; i < value.length(); ++i) {
				if (value[i] == '%') {
					char c;
					if (sscanf(value.substr(i + 1, 2).c_str(), "%2hhx", &c) == 1) {
						value[i] = c;
						value.erase(i + 1, 2);
					}
				}
			}
            postData[key] = value;
        }
    }
    return (postData);
}

// Generate HTML for guestbook
std::string	HttpRes::generateGuestbookHTML(void) {
    std::ostringstream html;
    html << "<!DOCTYPE html><html><head><title>Guestbook</title></head><body>";
    html << "<h1>Welcome to the Guestbook</h1>";
    html << "<form method='POST' action='/guestbook.html'>"
         << "Name: <input type='text' name='name'><br>"
         << "Message: <textarea name='message'></textarea><br>"
         << "<button type='submit'>Submit</button></form><hr>";
    html << "<h2>Messages</h2>";

	// Load guestbook entries from file
	std::ifstream file(GUESTBOOK_FILE.c_str());
	if (file) {
		std::string line;
		while (std::getline(file, line)) {
			size_t sep = line.find('|');
			if (sep != std::string::npos) {
				html << "<p><strong>" << line.substr(0, sep) << ":</strong> " << line.substr(sep + 1) << "</p>";
			}
		}
	}

    html << "</body></html>";
    return (html.str());
}

// Save a new entry to the file
void	HttpRes::saveEntry(const std::string& name, const std::string& message) {
    if (name.empty() || message.empty()) {
		return;
	}
	if (name.find('|') != std::string::npos || message.find('|') != std::string::npos) {
		fprintf(stderr, "Warning: Invalid characters in guestbook entry.\n");
		return;
	}
	if (name.length() > 100 || message.length() > 1000) {
		fprintf(stderr, "Warning: Guestbook entry too long.\n");
		return;
	}

	// Append entry to file	
	std::ofstream file(GUESTBOOK_FILE.c_str(), std::ios::app); // Append mode
	if (file) {
		file << name << "|" << message << "\n";
	} else {
		std::cerr << "Error: Could not open file " << GUESTBOOK_FILE << " for writing.\n";
	}
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
		if (_target == "/guestbook.html") {
			_contentType = determineContentType(_target);
			_body = generateGuestbookHTML();
			contentLength = _body.length();
			return;
		}
		if (access(("data/www" + _target).c_str(), F_OK) == -1) {
			_httpStatus = 404;
			return;
		} else if (access(("data/www" + _target).c_str(), R_OK) == -1) {
			_httpStatus = 403;
			return;
		}
		if (_target == "/") {
			_target = "/index.html";
		}
		_contentType = determineContentType(_target);
		_body = parseFile(_target);
		contentLength = _body.length();
    } else if (httpRequest.getMethod() == "POST" && _target == "/guestbook.html") {
        // Parse POST data
        if (httpRequest.getBodySize() > 0) {
            std::map<std::string, std::string> formData = parsePostData(httpRequest.getBody());

            if (formData.count("name") && formData.count("message")) {
                saveEntry(formData["name"], formData["message"]);
				printf("Saved entry: %s: %s\n", formData["name"].c_str(), formData["message"].c_str());
            }
        }

        // Redirect back to guestbook
		_httpStatus = 303;
        // response << "HTTP/1.1 303 See Other\r\nLocation: /\r\n\r\n";

    } else {
        _httpStatus = 405;
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
