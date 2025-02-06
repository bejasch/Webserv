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

void	HttpRes::getNameCookie(HttpReq &httpRequest) {
	if (httpRequest.getHeaders().find("cookie") != httpRequest.getHeaders().end()) {
		std::string cookie = httpRequest.getHeader("cookie");
		if (cookie.substr(0, 10) == "user_name=")
			_userName = cookie.substr(10);
	}
}

void	HttpRes::handleRequest(HttpReq &httpRequest, Server &server) {
	_server = &server;
	_target = httpRequest.getTarget();
	_httpStatus = httpRequest.getHttpStatus();
    if (_httpStatus >= 400 && _httpStatus < 600) {
		return;
	}
	_method = httpRequest.getMethod();
	getNameCookie(httpRequest);

	// Check if the method is allowed for the target (in routes)
	_route = server.getConfig()->getRouteForTarget(_target);
	if (_route) {
		std::cout << "Route found for target: " << _target << " with RootDir" << _route->getRootDirRoute() << std::endl;
		if (_route->getRootDirRoute() != "") {
			httpRequest.setRootDirReq(_route->getRootDirRoute());
		}
		//TODO: if allowed methos empty, then allow all methods specified in the server
		if (!_route->allowsMethod(_method)) {
			_httpStatus = 405;
			return;
		}
	}
	if (!_route) {
		_httpStatus = 404;
		return;
	}
	
    if (_method == "GET")
		GET(httpRequest);
	else if (_method == "POST")
		POST(httpRequest);
    else if  (_method == "DELETE")
		DELETE(server.getConfig()->getRootDirConfig() + _target);
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

void	HttpRes::GET(HttpReq &httpRequest) {
	if (_target == "/guestbook.html") {
		_contentType = "text/html";
		_body = generateGuestbookHTML(_userName);
		std::cout << "Generated guestbook page in GET" << std::endl;
		return;
	}
	// if (_target == "/")
	// 	_target = _server->getConfig()->getDefaultFile();
	if (_target.find(".py") != std::string::npos || _target.find(".php") != std::string::npos) {
		CGI cgi;
		std::cout << "Executing CGI script: " << _target << std::endl;
		_body = cgi.executeCGI_GET(httpRequest);
		_contentType = "text/html";
		return;
	}
	// Check if the target is a directory
	// std::string path = _server->getConfig()->getRootDirConfig() + _target;
	std::string	path = _route->getRootDirRoute() + _target;
	// TODO: nicht ganz richtig, da zB /images mit /data/images ersetzt wird 
	std::cout << "GET path: " << path << std::endl;
	if (isDirectory(path)) {
		if (_route->getAutoindex()) {
			_httpStatus = 200;
			_contentType = "text/html";
			generateAutoindexPage(path);
			return;
		} else if (!_route->getIndexFile().empty() && access((path + _route->getIndexFile()).c_str(), R_OK) != -1) {
			_target += _route->getIndexFile();
			_httpStatus = 200;
		} else if (!_route->getRedirectUrl().empty() && access((path + _route->getRedirectUrl()).c_str(), R_OK) != -1) {
			_target = _route->getRedirectUrl();
			_httpStatus = _route->getRedirectStatus();
		} else {
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

void HttpRes::POST(HttpReq &httpRequest) {
    std::cout << "POST DATA" << httpRequest.getBody() << std::endl;
    if (_target == "/guestbook.html") {
        if (!httpRequest.getBody().empty()) {
            std::map<std::string, std::string> formData = parsePostData(httpRequest.getBody());
            if (formData.count("name") && formData.count("message")) {
                // Check if this is a Scramble request
                if (formData.count("action") && (formData["action"] == "Scramble" || formData["action"] == "Capitalize")) {
					std::cout << "CGI POST request" << std::endl;
                    CGI cgi;
                    std::string Message = cgi.executeCGI_POST(httpRequest, formData);
                    if (Message != "500") {
                        saveGuestbookEntry(formData["name"], Message);
                    }
                } else {
                    // Regular submission
                    saveGuestbookEntry(formData["name"], formData["message"]);
                }
                std::cout << "Saved entry: " << formData["name"] << ": " << formData["message"] << std::endl;
            }
        }
        _httpStatus = 303;  // Redirect after POST
        _target = "/guestbook.html";  // Redirect back to guestbook
        return;
    }
    std::cout << "POST target: " << _target << std::endl;
    std::string path = _server->getConfig()->getRootDirConfig() + _target;
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
