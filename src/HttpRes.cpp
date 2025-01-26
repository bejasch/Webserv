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
				std::cerr << "Error: Failed to write to socket after " << MAX_RETRY_COUNT << " retries.\n";
				break;
			}
			std::cerr << "Warning: Write failed, retrying (" << retry_count << "/" << MAX_RETRY_COUNT << ")...\n";
			continue;
		}
		retry_count = 0;
		total_sent += sent;
	}
	if (total_sent < size)
		std::cerr << "Warning: Only " << total_sent << " out of " << size << " bytes were sent.\n";
	else
		std::cout << "Successfully sent " << total_sent << " bytes to client.\n";
}


// routes_expl = {
// 	{"/", "GET"},
// 	{"/index.html", "GET"},
// 	{"/data/", "GET"},
// 	{"/static/", "POST"}
// };
// target_expl = {
// 	{"/", "GET"},
// 	{"/index.html", "GET"},
// 	{"/static/guestbook.html", "GET"},
// 	{"/guestbook.html", "POST"}
// };


void	HttpRes::generateAutoindexPage(const std::string &path) {
    _body = "<html><head><title>Index of " + _target + "</title></head><body>";
    _body += "<h1>Index of " + _target + "</h1><ul>";
    
	DIR	*dir = opendir(path.c_str());
	if (dir) {
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL) {
			std::string	entryName(entry->d_name);
			if (entryName[0] == '.')
				continue;
			_body += "<li><a href=\"" + _target + "/" + entryName + "\">" + entryName + "</a></li>";
		}
		closedir(dir);
	}
    _body += "</ul></body></html>";
}

void	HttpRes::GET(HttpReq &httpRequest, Server &server, Route *route) {
	if (_target == "/guestbook.html") {
		_contentType = "text/html";
		_body = generateGuestbookHTML();
		return;
	}
	if (_target == "/")
		_target = server.getConfig()->getDefaultFile();
	if (_target.find(".py") != std::string::npos) {
		std::cout << "Executing CGI script: " << _target << std::endl;
		_body = server.getConfig()->getCGI()->executeCGI(httpRequest, server);
		_contentType = "text/html";
		return;
	}
	// Check if the target is a directory
	std::string path = server.getConfig()->getRootDir() + _target;
	if (isDirectory(path)) {
		if (route && route->getIndexFile() != "" && access((path + route->getIndexFile()).c_str(), R_OK) != -1) {
			_target += route->getIndexFile();
		} else {
			std::cout << "Path: " << path << std::endl;
			if (route && route->getAutoindex()) {
				_httpStatus = 200;
				_contentType = "text/html";
				generateAutoindexPage(path);
			} else if (route && route->getRedirectUrl() != "") {	// serve the default file
				_target = route->getRedirectUrl();
				_httpStatus = route->getRedirectStatus();
			} else
				_httpStatus = 403;
			return;
		}
	} else if (access((path).c_str(), F_OK) == -1) {	// Check if the file exists ...
		_httpStatus = 404;
		return;
	} else if (access((path).c_str(), R_OK) == -1) {	// ...and is readable
		_httpStatus = 403;
		return;
	}
	determineContentType();
	parseFile(server);
}

void	HttpRes::POST(HttpReq &httpRequest, Server &server) {
	if (_target == "/guestbook.html") {
		if (!httpRequest.getBody().empty()) {
			std::map<std::string, std::string> formData = parsePostData(httpRequest.getBody());

			if (formData.count("name") && formData.count("message")) {
				saveGuestbookEntry(formData["name"], formData["message"]);
				std::cout << "Saved entry: " << formData["name"] << ": " << formData["message"] << std::endl;
			}
		}
		_httpStatus = 303;	// Redirect (see other)
	}
	if (_target.find(".py") != std::string::npos) {
		server.getConfig()->getCGI()->executeCGI(httpRequest, server);
		//TODO: execute script
	}
}

void	HttpRes::DELETE(const std::string &path) {
	if (deleteFileDir(path)) {
		_body = "Resource deleted.\n";
		_httpStatus = 200;
	} else {
		_httpStatus = 404;
	}
}

void	HttpRes::handleRequest(HttpReq &httpRequest, Server &server) {
	_protocol = httpRequest.getProtocol();
	_httpStatus = httpRequest.getHttpStatus();
	_statusMessage = statusMessages[_httpStatus];
	_target = httpRequest.getTarget();
    if (_httpStatus >= 400 && _httpStatus < 600) {
		return;
	}
	_method = httpRequest.getMethod();

	// Check if the method is allowed for the target (in routes)
	Route *route = server.getConfig()->getRouteForTarget(_target);
	if (route) {
		//TODO: if allowed methos empty, then allow all methods specified in the server
		if (!route->allowsMethod(_method)) {
			_httpStatus = 405;
			return;
		}
	}
	
    if (_method == "GET")
		GET(httpRequest, server, route);
	else if (_method == "POST")
		POST(httpRequest, server);
    else if  (_method == "DELETE")
		DELETE(server.getConfig()->getRootDir() + _target);
    else						// Unsupported method
		_httpStatus = 405;
}


void	HttpRes::determineContentType(void) {
	std::string extension;
	try
	{
		extension = _target.substr(_target.find_last_of(".") + 1);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		_contentType = "text/plain";
		return;
	}

	// Look up the MIME type in the static map
	std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);

	if (it != mimeTypes.end())
		_contentType = it->second;
	else
		_contentType = "text/plain";
}

bool	HttpRes::parseFile(Server &server) {
    std::ifstream file((server.getConfig()->getRootDir() + _target).c_str());
    if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << _target << std::endl;
		_httpStatus = 404;
        return (false);
    }
    std::stringstream buffer;
    buffer << file.rdbuf(); // Read entire file
    _body = buffer.str();
    file.close();
    return (true);
}

void HttpRes::writeResponse(int client_fd) {
    if (_httpStatus >= 400 && _httpStatus < 600)
		return	(generateErrorResponse(client_fd));
	
	// Build the status line
    std::ostringstream	response_stream;
    response_stream << _protocol << " " << _httpStatus << " " << _statusMessage << "\n";

    // Add headers
    response_stream << "Content-Type: " << _contentType << "\n";
	response_stream << "Location: " << _target << "\n";
    response_stream << "Content-Length: " << _body.length() << "\n\n";
    // response_stream << "Connection: keep-alive\n\n"; //without this we still be stuck in the loop (still issues here)

    // Add body
    response_stream << _body;

	sendResponse(client_fd, response_stream.str());
}
