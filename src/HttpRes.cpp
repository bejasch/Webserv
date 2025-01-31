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
		GET(httpRequest, server, route);
	else if (_method == "POST")
		POST(httpRequest, server);
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
		std::string emptyArgs = "";
		_body = server.getConfig()->getCGI()->executeCGI(httpRequest, server, emptyArgs);
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
		return;
	}
	if (_target.find(".py") != std::string::npos) {
		std::string args = httpRequest.getBody();
		std::cout << "Executing CGI script with args" << args << std::endl;
		server.getConfig()->getCGI()->executeCGI(httpRequest, server, args);
		//TODO: CGI POST
	}
	// Check if the target exists
	std::string path = server.getConfig()->getRootDir() + _target;
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

std::string	HttpRes::getResponse(void) {
    if (_httpStatus >= 400 && _httpStatus < 600)
		generateErrorBody();
	
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
