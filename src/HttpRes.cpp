#include "../headers/AllHeaders.hpp"


HttpRes::HttpRes() : _httpStatus(0), _responseSize(0) {
    std::cout << "HttpRes default constructor called" << std::endl;
}

HttpRes::HttpRes(const HttpRes &other) : _method(other._method),
	_httpStatus(other._httpStatus),	_responseSize(other._responseSize),
	_target(other._target), _contentType(other._contentType),
	_body(other._body) {}

HttpRes HttpRes::operator=(const HttpRes &another) {
	if (this == &another)
		return (*this);
	_method = another._method;
	_httpStatus = another._httpStatus;
	_responseSize = another._responseSize;
	_target = another._target;
	_contentType = another._contentType;
	_body = another._body;

	return (*this);
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

std::map<int, std::string> HttpRes::statusDescription = {
	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{204, "No Content"},
	{206, "Partial Content"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{303, "See Other"},
	{304, "Not Modified"},
	{307, "Temporary Redirect"},
	{308, "Permanent Redirect"},
	{400, "Bad Request"},
	{401, "Unauthorized"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{408, "Request Timeout"},
	{411, "Length Required"},
	{413, "Payload Too Large"},
	{415, "Unsupported Media Type"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{503, "Service Unavailable"},
	{504, "Gateway Timeout"}
};

void	HttpRes::handleRequest(HttpReq &httpRequest, Server &server) {
	_server = &server;
	_httpStatus = httpRequest.getHttpStatus();
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
		GET(httpRequest, route);
	else if (_method == "POST")
		POST(httpRequest);
    else if  (_method == "DELETE")
		DELETE(server.getConfig()->getRootDir() + _target);
    else						// Unsupported method
		_httpStatus = 405;
}

void	HttpRes::generateErrorBody(void) {
	_contentType = "text/html";
	std::string statusMessage = statusDescription[_httpStatus];

	_body = "<html><head><title>" + std::to_string(_httpStatus) + " "
			+ statusMessage + "</title></head><body><h1>"
			+ std::to_string(_httpStatus) + " " + statusMessage + "</h1>";

	if (statusDescription.find(_httpStatus) != statusDescription.end())
		_body += "<p>" + statusDescription[_httpStatus] + "</p>";
	
	_body += "</body></html>";
}


void	HttpRes::generateAutoindexPage(const std::string &path) {
    _body = "<html><head><title>Index of " + _target + "</title></head><body>";
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

std::string	extract_cookie(HttpReq &httpRequest) {
	std::map<std::string, std::string>::const_iterator it = httpRequest.getHeaders().find("Cookie");
	if (it == httpRequest.getHeaders().end()) return "";
	std::string value = it->second;
	if (value.substr(11) != "session_id=")
		return "";
	size_t pos = value.find(";");
	if (pos == std::string::npos)
		return "";
	return value.substr(11, pos - 11);
}

// Function to generate a random session ID
std::string generate_session_id() {
    srand(time(NULL));
    std::stringstream ss;
    ss << rand() % 1000000;  // Simple random number as session ID
    return ss.str();
}

void	HttpRes::GET(HttpReq &httpRequest, Route *route) {
	if (_target == "/guestbook.html") {
		_contentType = "text/html";
		_body = generateGuestbookHTML();
		return;
	}
	if (_target == "/")
		_target = _server->getConfig()->getDefaultFile();
	if (_target.find(".py") != std::string::npos) {
		std::cout << "Executing CGI script: " << _target << std::endl;
		_body = _server->getConfig()->getCGI()->executeCGI(httpRequest, *_server);
		_contentType = "text/html";
		return;
	}
	if (_target == "/session.html") {
		_contentType = "text/html";
		_body = generateSessionHTML(httpRequest);
		return;
	}
	// Check if the target is a directory
	std::string path = _server->getConfig()->getRootDir() + _target;
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
	parseFile();
}



void	HttpRes::POST(HttpReq &httpRequest) {
	if (_target == "/guestbook.html") {
		if (!httpRequest.getBody().empty()) {
			std::map<std::string, std::string> formData = parsePostData(httpRequest.getBody());

			if (formData.count("name") && formData.count("message")) {
				saveGuestbookEntry(formData["name"], formData["message"]);
				std::cout << "Saved entry: " << formData["name"] << ": " << formData["message"] << std::endl;
			}
		}
		_httpStatus = 303;	// Redirect (see other)
		return;
	}
	if (_target.find(".py") != std::string::npos) {
		_server->getConfig()->getCGI()->executeCGI(httpRequest, *_server);
		//TODO: CGI POST
	}
	// Check if the target exists
	std::string path = _server->getConfig()->getRootDir() + _target;
	printf("\t-> Path: %s\n", path.c_str());
	if (access(path.c_str(), F_OK) == 0) {
		_httpStatus = 404;
		return;
	}
	if (saveFile(path, httpRequest.getBody().c_str(), httpRequest.getBody().size()))
		_httpStatus = 201;
	else
		_httpStatus = 500;
}

// gets the full path of the file to delete
void	HttpRes::DELETE(const std::string &path) {
	if (access(path.c_str(), F_OK) != 0) {			// Check if the file exists
		_httpStatus = 404;
		return;
	} else if (access(path.c_str(), W_OK) != 0) {	// Check if the file is writable
		_httpStatus = 403;
		return;
	}
	if (remove(path.c_str()) == 0) {				// Delete the file
		_httpStatus = 204;
	} else {
		std::cerr << "Error deleting file: " << strerror(errno) << std::endl;
		_httpStatus = 500;
	}
}

// void	HttpRes::DELETE(const std::string &path) {
// 	if (deleteFileDir(path)) {
// 		_body = "Resource deleted.\n";
// 		_httpStatus = 200;
// 	} else {
// 		_httpStatus = 404;
// 	}
// }

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

bool	HttpRes::parseFile(void) {
    std::ifstream file((_server->getConfig()->getRootDir() + _target).c_str());
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

std::string	HttpRes::getSessionResponse(void) {
	std::string cookie = extract_cookie(_server->getHttpReq());
	std::string session_id = _server->getConfig()->getSession()->getSessionId(cookie);
	std::string username = _server->getConfig()->getSession()->getUsername(session_id);
	std::string body = "<html><head><title>Session</title></head><body>";
	if (username.empty()) {
		body += "<h1>Session</h1><form action=\"/session.html\" method=\"post\">";
		body += "<label for=\"username\">Username:</label><br>";
		body += "<input type=\"text\" id=\"username\" name=\"username\"><br>";
		body += "<input type=\"submit\" value=\"Submit\">";
	} else {
		body += "<h1>Welcome, " + username + "!</h1>";
		body += "<form action=\"/session.html\" method=\"post\">";
		body += "<input type=\"hidden\" name=\"logout\" value=\"true\">";
		body += "<input type=\"submit\" value=\"Logout\">";
	}
	body += "</form></body></html>";
	return (body);
}

// Function to handle HTTP requests and manage sessions
std::string handle_request(const std::string& request) {
    std::string session_id = extract_cookie(request, "session_id");
    std::string response;

    if (session_id.empty() || session_store.find(session_id) == session_store.end()) {
        // Create a new session
        session_id = generate_session_id();
        session_store[session_id] = "Hello, new user!";

        response = "HTTP/1.1 200 OK\r\n"
                   "Set-Cookie: session_id=" + session_id + "; Path=/; HttpOnly\r\n"
                   "Content-Type: text/html\r\n\r\n"
                   "<html><body>New session created! Your session ID: " + session_id + "</body></html>";
    } else {
        // Retrieve session data
        std::string user_data = session_store[session_id];

        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html\r\n\r\n"
                   "<html><body>Welcome back! Your session ID: " + session_id + "<br>"
                   "Your session data: " + user_data + "</body></html>";
    }

    return response;
}

std::string	HttpRes::getResponse(void) {
    if (_httpStatus >= 400 && _httpStatus < 600) {
		// Store a reference to the map
		const std::map<int, std::string> &errorPages = _server->getConfig()->getErrorPages();

		// Now use the reference for finding the element
		std::map<int, std::string>::const_iterator it = errorPages.find(_httpStatus);
		if (it != errorPages.end()) {
			std::ifstream file((_server->getConfig()->getRootDir() + it->second).c_str());
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
	if (_target == "/session.html")
		return (getSessionResponse());

	std::string response;
	response = "HTTP/1.1 " + std::to_string(_httpStatus) + " " + statusDescription[_httpStatus] + "\r\n";
	response += "Content-Type: " + _contentType + "\r\n";
	response += "Location: " + _target + "\r\n";
	response += "Content-Length: " + std::to_string(_body.length()) + "\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += _body;

	_responseSize = response.size();
	return (response);
}

size_t	HttpRes::getResponseSize(void) const {
	return (_responseSize);
}
