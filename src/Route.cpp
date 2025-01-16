#include "../headers/AllHeaders.hpp"

Route::Route() {
    this->path = "";
    this->allowed_methods = {""};
    this->root_dir = "";
    this->index_file = "";
    this->autoindex = false;
    this->redirect_status = 000;
    this->redirect_url = "";
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

void Route::printRoute() {
    std::cout << "path: " << path << std::endl;
    for (int i = 0; i < allowed_methods.size(); i++) {
        std::cout << "allowed_methods: " << allowed_methods[i] << std::endl;
    }
    std::cout << "root_dir: " << root_dir << std::endl;
    std::cout << "index_file: " << index_file << std::endl;
    std::cout << "autoindex: " << autoindex << std::endl;
    std::cout << "redirect_status: " << redirect_status << std::endl;
    std::cout << "redirect_url: " << redirect_url << std::endl;
}