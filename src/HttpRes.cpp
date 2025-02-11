#include "../headers/AllHeaders.hpp"

HttpRes::HttpRes(void) : _server(NULL), _route(NULL), _method(""), _httpStatus(0), _responseSize(0),
	_userName(""), _target(""), _serverPath(""), _contentType(""), _body(""), _wasRedirected(false) {
	// std::cout << "HttpRes default constructor called" << std::endl;
}

HttpRes::HttpRes(const HttpRes &other) : _server(other._server), _route(other._route), _method(other._method),
	_httpStatus(other._httpStatus), _responseSize(other._responseSize), _userName(other._userName),
	_target(other._target), _serverPath(other._serverPath), _contentType(other._contentType),
	_body(other._body), _wasRedirected(other._wasRedirected) {
	// std::cout << "HttpRes copy constructor called" << std::endl;
}

HttpRes HttpRes::operator=(const HttpRes &another) {
	if (this == &another)
		return (*this);
	_server = another._server;
	_route = another._route;
	_method = another._method;
	_httpStatus = another._httpStatus;
	_responseSize = another._responseSize;
	_userName = another._userName;
	_target = another._target;
	_serverPath = another._serverPath;
	_contentType = another._contentType;
	_body = another._body;
	_wasRedirected = another._wasRedirected;
	return (*this);
}

HttpRes::~HttpRes(void) {
	// std::cout << "HttpRes destructor called" << std::endl;
}

// Function to initialize mimeTypes
const std::string	&HttpRes::getMimeType(const std::string &extension) {
	static std::map<std::string, std::string> mimeTypes;
	if (mimeTypes.empty()) {
		mimeTypes.insert(std::make_pair("html", "text/html"));
		mimeTypes.insert(std::make_pair("css", "text/css"));
		mimeTypes.insert(std::make_pair("js", "text/javascript"));
		mimeTypes.insert(std::make_pair("jpg", "image/jpeg"));
		mimeTypes.insert(std::make_pair("jpeg", "image/jpeg"));
		mimeTypes.insert(std::make_pair("png", "image/png"));
		mimeTypes.insert(std::make_pair("gif", "image/gif"));
		mimeTypes.insert(std::make_pair("ico", "image/x-icon"));
	}
	if (mimeTypes.find(extension) != mimeTypes.end())
		return (mimeTypes[extension]);
	return (mimeTypes["html"]);
}

// Function to initialize statusDescription
const std::string	&HttpRes::getStatusDescription(int status) {
	static std::map<int, std::string> statusDescription;
	if (statusDescription.empty()) {
		statusDescription.insert(std::make_pair(200, "OK"));
		statusDescription.insert(std::make_pair(201, "Created"));
		statusDescription.insert(std::make_pair(202, "Accepted"));
		statusDescription.insert(std::make_pair(204, "No Content"));
		statusDescription.insert(std::make_pair(206, "Partial Content"));
		statusDescription.insert(std::make_pair(301, "Moved Permanently"));
		statusDescription.insert(std::make_pair(302, "Found"));
		statusDescription.insert(std::make_pair(303, "See Other"));
		statusDescription.insert(std::make_pair(304, "Not Modified"));
		statusDescription.insert(std::make_pair(307, "Temporary Redirect"));
		statusDescription.insert(std::make_pair(308, "Permanent Redirect"));
		statusDescription.insert(std::make_pair(400, "Bad Request"));
		statusDescription.insert(std::make_pair(401, "Unauthorized"));
		statusDescription.insert(std::make_pair(403, "Forbidden"));
		statusDescription.insert(std::make_pair(404, "Not Found"));
		statusDescription.insert(std::make_pair(405, "Method Not Allowed"));
		statusDescription.insert(std::make_pair(408, "Request Timeout"));
		statusDescription.insert(std::make_pair(411, "Length Required"));
		statusDescription.insert(std::make_pair(413, "Payload Too Large"));
		statusDescription.insert(std::make_pair(415, "Unsupported Media Type"));
		statusDescription.insert(std::make_pair(500, "Internal Server Error"));
		statusDescription.insert(std::make_pair(501, "Not Implemented"));
		statusDescription.insert(std::make_pair(502, "Bad Gateway"));
		statusDescription.insert(std::make_pair(503, "Service Unavailable"));
		statusDescription.insert(std::make_pair(504, "Gateway Timeout"));
	}
	if (statusDescription.find(status) != statusDescription.end())
		return (statusDescription[status]);
	return (statusDescription[404]);
}

void	HttpRes::getNameCookie(HttpReq &httpRequest) {
	if (httpRequest.getHeaders().find("cookie") != httpRequest.getHeaders().end()) {
		std::string cookie = httpRequest.getHeader("cookie");
		if (cookie.substr(0, 10) == "user_name=")
			_userName = cookie.substr(10);
	}
}

void	HttpRes::handleRequest(HttpReq &httpRequest, Server &server, int client_fd) {
	_server = &server;
	_target = httpRequest.getTarget();
	_httpStatus = httpRequest.getHttpStatus();
	_client_fd = client_fd;
	if (_httpStatus >= 400 && _httpStatus < 600) {
		return;
	}
	_method = httpRequest.getMethod();
	getNameCookie(httpRequest);					// Get the name of the user from the cookie

	_route = server.getConfig()->getRouteForTarget(_target);
	if (!_route || _route->getRootDirRoute().empty()) {
		_httpStatus = 404;
		return;
	} else if (!_route->allowsMethod(_method)) {
		_httpStatus = 405;
		return;
	}
	_serverPath = resolvePath(_target, _route->getPath(), _route->getRootDirRoute());
	std::cout << "Server path: " << _serverPath << std::endl;
	
	if (_method == "GET")
		GET();
	else if (_method == "POST")
		POST(httpRequest);
	else if  (_method == "DELETE")
		DELETE();
	else						// Unsupported method
		_httpStatus = 405;
}

void	HttpRes::generateErrorBody(void) {
	_contentType = "text/html";
	std::string statusMessage = getStatusDescription(_httpStatus);

	_body = "<html><head><title>" + intToString(_httpStatus) + " "
		+ statusMessage + "</title></head><body><h1>"
		+ intToString(_httpStatus) + " " + statusMessage + "</h1>"
		+ "<p>" + statusMessage + "</p></body></html>";
}

void	HttpRes::generateAutoindexPage(const std::string &path) {
	_body = "<html><head><title>Index of " + _target + "</title>";
	_body += "<style>";
	_body += "body { font-family: Arial, sans-serif; background-color: #ffffcc; padding: 20px; }";
	_body += "h1 { color:rgb(255, 98, 0); }"; // "Index of" in orange
	_body += "ul { list-style-type: disc; padding-left: 20px; }";
	_body += "li { margin: 5px 0; }";
	_body += "a { text-decoration: none; color:rgb(17, 0, 255); font-weight: bold; }";
	_body += "a:hover { text-decoration: underline; }";
	_body += "button { background-color:rgb(49, 146, 250); color: white; border: none; padding: 8px 12px;";
	_body += "border-radius: 5px; cursor: pointer; font-size: 14px; }";
	_body += "#login_status { position: fixed; top: 10px; right: 10px; color: white; padding: 12px 18px;";
	_body += "border-radius: 20px; font-size: 16px; box-shadow: 3px 3px 12px rgba(0, 0, 0, 0.3); font-weight: bold; }";
	_body += "</style></head><body>";

	// Login status box
	if (!_userName.empty()) {
		_body += "<div id='login_status' style='background-color: green;'>Logged in as " + _userName + "</div>";
	} else {
		_body += "<div id='login_status' style='background-color: red;'>Not logged in</div>";
	}
	_body += "<button onclick=\"window.location.href='/index.html'\">Back to Main Page</button>";
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

void	HttpRes::GET(void) {
	if (_target == "/guestbook.html") {
		_contentType = "text/html";
		_body = generateGuestbookHTML(_userName);
		return;
	}
	if (_route->getPath().find(".py") != std::string::npos || _route->getPath().find(".php") != std::string::npos) {
		CGI cgi;
		_body = cgi.executeCGI_GET(*this, _client_fd);
		_contentType = "text/html";
		return;
	}
	std::cout << "Server path: " << _serverPath << std::endl;
	if (!_wasRedirected && !_route->getRedirectUrl().empty()) {		// return directive
		_wasRedirected = true;
		_target = _route->getRedirectUrl();
		_httpStatus = _route->getRedirectStatus();
		_route = _server->getConfig()->getRouteForTarget(_route->getRedirectUrl());
		_serverPath = _route->getRootDirRoute() + _target;
		return (GET());
	}
	if (isDirectory(_serverPath)) {
		if (_route->getAutoindex()) {											// autoindex directive
			_httpStatus = 200;
			_contentType = "text/html";
			generateAutoindexPage(_serverPath);
			return;
		} else if (!_wasRedirected && !_route->getIndexFile().empty()) {		// index-file directive
			std::cout << "Redirecting to index file" << std::endl;
			_wasRedirected = true;
			_target = _route->getIndexFile();
			_route = _server->getConfig()->getRouteForTarget(_route->getIndexFile());
			_serverPath = _route->getRootDirRoute() + _target;
			return (GET());
		} else {																// No index-file or autoindex
			_httpStatus = 403;
			return;
		}
	} else if (access((_serverPath).c_str(), F_OK) == -1) {	// Check if the file exists ...
		_httpStatus = 404;
		return;
	} else if (access((_serverPath).c_str(), R_OK) == -1) {	// ...and is readable
		_httpStatus = 403;
		return;
	}
	determineContentType();
	parseFile();
}

void HttpRes::POST(HttpReq &httpRequest) {
	if (_target == "/guestbook.html") {
		if (!httpRequest.getBody().empty()) {
			std::map<std::string, std::string> formData = parsePostData(httpRequest.getBody());
			if (formData.count("name") && formData.count("message")) {
				// Check if this is a Scramble request
				if (formData.count("action") && (formData["action"] == "Scramble.py" || formData["action"] == "Capitalize.php")) {
					std::cout << "CGI POST request" << std::endl;
					CGI cgi;
					std::string Message = cgi.executeCGI_POST(*this, formData);
					if (Message != "500") {
						saveGuestbookEntry(formData["name"], Message);
					}
				} else {
					// Regular submission
					saveGuestbookEntry(formData["name"], formData["message"]);
				}
				std::cout << "Saved entry: " << formData["name"] << ": " << formData["message"] << std::endl;
			}
			else {
				_httpStatus = 400;
				return;
			}
		}
		_httpStatus = 303;			// Redirect after POST
		return;
	}
	if (!access(_route->getRootDirRoute().c_str(), W_OK) && !access(_serverPath.c_str(), F_OK)) {
		_httpStatus = 404;
		return;
	}
	if (saveFile(_serverPath, httpRequest.getBody().c_str(), httpRequest.getBody().size()))
		_httpStatus = 201;
	else
		_httpStatus = 500;
}

// gets the full path of the file to delete
void	HttpRes::DELETE(void) {
	if (access(_serverPath.c_str(), F_OK)) {			// Check if the file exists
		_httpStatus = 404;
		return;
	} else if (access(_serverPath.c_str(), W_OK)) {		// Check if the file is writable
		_httpStatus = 403;
		return;
	}
	if (remove(_serverPath.c_str()) == 0) {				// Delete the file
		_httpStatus = 204;
	} else {
		std::cerr << "Error deleting file: " << strerror(errno) << std::endl;
		_httpStatus = 500;
	}
}

void	HttpRes::determineContentType(void) {
	try
	{
		_contentType = getMimeType(_target.substr(_target.find_last_of(".") + 1));
	}
	catch(const std::exception& e)
	{
		_contentType = getMimeType("html");
	}
}

bool	HttpRes::parseFile(void) {
	std::ifstream file((_server->getConfig()->getRootDirConfig() + _target).c_str());
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

std::string	HttpRes::getResponse(void) {
	if (_httpStatus >= 400 && _httpStatus < 600) {
		// Store a reference to the map
		const std::map<int, std::string> &errorPages = _server->getConfig()->getErrorPages();

		// Now use the reference for finding the element
		std::map<int, std::string>::const_iterator it = errorPages.find(_httpStatus);
		if (it != errorPages.end()) {
			std::ifstream file((_server->getConfig()->getRootDirConfig() + it->second).c_str());
			if (!file.is_open()) {
				generateErrorBody();
			} else {
				std::stringstream buffer;
				buffer << file.rdbuf();
				_body = buffer.str();
				file.close();
				_contentType = "text/html";
			}
		} else {
			generateErrorBody();
		}
	}
	std::string response;
	response = "HTTP/1.1 " + intToString(_httpStatus) + " " + getStatusDescription(_httpStatus) + "\r\n";
	response += "Content-Type: " + _contentType + "\r\n";
	response += "Location: " + _target + "\r\n";
	response += "Content-Length: " + intToString(_body.length()) + "\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += _body;

	_responseSize = response.size();
	return (response);
}

size_t	HttpRes::getResponseSize(void) const {
	return (_responseSize);
}

const std::string	&HttpRes::getTarget(void) const {
	return (_target);
}

const std::string	&HttpRes::getMethod(void) const {
	return (_method);
}

Route				*HttpRes::getRoute(void) const {
	return (_route);
}

void	HttpRes::setStatus(int status) {
	_httpStatus = status;
}

Server	*HttpRes::getServer(void) const {
	return (_server);
}
