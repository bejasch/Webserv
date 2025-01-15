#include "../headers/AllHeaders.hpp"

Route::Route() {
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

void Route::setRootDir(const std::string &root_dir) {
    this->root_dir = root_dir;
}

void Route::setIndexFile(const std::string &index_file) {
    this->index_file = index_file;
}

void Route::setAutoindex(std::string autoindex) {
    if (autoindex == "on")
        this->autoindex = true;
    else
        this->autoindex = false;
}

void Route::setRedirectStatus(int redirect_status) {
    this->redirect_status = redirect_status;
}

void Route::setRedirectUrl(const std::string &redirect_url) {
    this->redirect_url = redirect_url;
}

void Route::printRoute(const Route &route) {
    std::cout << "path: " << route.path << std::endl;
    std::cout << "allowed_methods1: " << route.allowed_methods[0] << std::endl;
    std::cout << "allowed_methods2: " << route.allowed_methods[1] << std::endl;
    std::cout << "root_dir: " << route.root_dir << std::endl;
    std::cout << "index_file: " << route.index_file << std::endl;
    std::cout << "autoindex: " << route.autoindex << std::endl;
    std::cout << "redirect_status: " << route.redirect_status << std::endl;
    std::cout << "redirect_url: " << route.redirect_url << std::endl;
}