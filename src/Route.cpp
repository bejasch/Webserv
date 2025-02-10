#include "../headers/AllHeaders.hpp"

// TODO: Cant there be many redirect pages?
Route::Route() 
    : path(""),
      allowed_methods(),
      root_dir(""),
      index_file(""),
      autoindex(false),
      redirect_status(0),
      redirect_url(""),
      autoindex_set(false) {
    std::cout << "Route default constructor called" << std::endl;
}

Route::~Route() {
    std::cout << "Route destructor called" << std::endl;
}

void Route::setPath(const std::string &path) {
    this->path = path;
}

void Route::setAllowedMethods(const std::vector<std::string> &allowed_methods) {
    this->allowed_methods = allowed_methods;
}

void Route::setRootDirRoute(const std::string &root_dir) {
    this->root_dir = root_dir;
}

void Route::setIndexFile(const std::string &index_file) {
    this->index_file = index_file;
}

void Route::setAutoindex(bool autoindex) {
    this->autoindex = autoindex;
    this->autoindex_set = true; 
}

void Route::setRedirectStatus(int redirect_status) {
    this->redirect_status = redirect_status;
}

void Route::setRedirectUrl(const std::string &redirect_url) {
    this->redirect_url = redirect_url;
}

bool	Route::containsTarget(const std::string &target) const {
	return (target.find(path) == 0); // if Route matches at the beginning
}

bool	Route::allowsMethod(const std::string &method) const {
	return (std::find(allowed_methods.begin(), allowed_methods.end(), method) != allowed_methods.end());
}

void Route::printRoute() {
    std::cout << "\npath: " << path << std::endl;
    for (unsigned long i = 0; i < allowed_methods.size(); i++) {
        std::cout << "allowed_methods: " << allowed_methods[i] << std::endl;
    }
    std::cout << "root_dir: " << root_dir << std::endl;
    std::cout << "index_file: " << index_file << std::endl;
    std::cout << "autoindex: " << autoindex << std::endl;
    std::cout << "redirect_status: " << redirect_status << std::endl;
    std::cout << "redirect_url: " << redirect_url << std::endl;
}

void Route::cleanRoute(Route *route) {
    route->setAllowedMethods(std::vector<std::string>());
    route->setRootDirRoute("");
    route->setIndexFile("");
    route->setAutoindex(false);
    route->setRedirectStatus(0);
    route->setRedirectUrl("");    
}

//TODO: implement this if needed
int Route::checkRoute(Route *route)
{
	(void)route;
    return (0);
}
