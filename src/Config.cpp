#include "../headers/AllHeaders.hpp"

Config::Config() 
    : server_port(0),
      server_name(""),
      root_dir(""),
      max_body_size(0),
      default_file(""),
      error_pages({}),
      routes({}),
      cgi(NULL) {
    std::cout << "Config default constructor called" << std::endl;
}

Config::~Config() {
    std::cout << "Config destructor called" << std::endl;
}

void Config::printConfig() {
    std::cout << "\nPrinting config" << std::endl;
    std::cout << "server_port: " <<  server_port << std::endl;
    std::cout << "server_name: " << server_name << std::endl;
    std::cout << "root_dir: " << root_dir << std::endl;
    std::cout << "max_body_size: " << max_body_size << std::endl;
    std::cout << "default_file: " << default_file << std::endl;
    for (std::map<int, std::string>::iterator it = error_pages.begin(); it != error_pages.end(); ++it) {
        std::cout << "error_status: " << it->first << " error_file: " << it->second << std::endl;
    }
    for (int i = 0; i < allowed_methods.size(); i++) {
        std::cout << "allowed_methods: " << allowed_methods[i] << std::endl;
    }
    if (cgi)
        cgi->printCGI();
    for (int i = 0; i < routes.size(); i++) {
        std::cout << "\nRoute " << i << std::endl;
        routes[i]->printRoute();
    }
}

Route	*Config::getRouteForTarget(std::string &target) {
	Route	*route = NULL;
	for (int i = 0; i < routes.size(); i++) {
		if (routes[i]->containsTarget(target)) {
			if (!route)
				route = routes[i];
			else if (routes[i]->getPath().length() > route->getPath().length())
				route = routes[i];
		}
	}
	return (route);
}

void Config::setPort(int port) {
    this->server_port = port;
}

void Config::setName(std::string name) {
    this->server_name = name;
}

void Config::setRootDir(std::string root_dir) {
    this->root_dir = root_dir;
}

void Config::setMaxBodySize(int max_body_size) {
    this->max_body_size = max_body_size;
}

void Config::setDefaultFile(std::string default_file) {
    this->default_file = default_file;
}

void Config::setErrorPage(int error_status, std::string error_file) {
    this->error_pages[error_status] = error_file;
}

void Config::setAllowedMethods(const std::vector<std::string> &allowed_methods) {
    this->allowed_methods = allowed_methods;
}

void Config::addRoute(Route *route) {
    routes.push_back(route);
}

void Config::addCGI(CGI *cgi) {
    this->cgi = cgi;
}

void Config::freeConfig() {
    if (!routes.empty()) {
        for (int i = 0; i < routes.size(); i++) {
            delete routes[i];
        }
        routes.clear();
    }
    if (cgi)
        delete cgi;
}

int Config::initialisedCheck() {
    if (server_port == 0) {
        std::cerr << "Server port not set" << std::endl;
        return (1);
    }
    if (server_name == "") {
        std::cerr << "Server name not set" << std::endl;
        return (1);
    }
    if (root_dir == "") {
        std::cerr << "Root directory not set" << std::endl;
        return (1);
    }
    if (max_body_size == 0) {
        std::cerr << "Max body size not set" << std::endl;
        return (1);
    }
    if (default_file == "") {
        std::cerr << "Default file not set" << std::endl;
        return (1);
    }
    if (error_pages.empty()) {
        std::cerr << "Error pages not set" << std::endl;
        return (1);
    }
    if (allowed_methods.empty()) {
        std::cerr << "Allowed methods not set" << std::endl;
        return (1);
    }
    return (0);
}
