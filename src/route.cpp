#include "../headers/route.hpp"

Route::Route() {
    this->path = "/info/";
    this->allowed_methods = {"GET", "POST", "DELETE"};
    this->root_dir = "";
    this->index_file = "www/info.html";
    this->autoindex = false;
    this->redirect_status = 0;
    this->redirect_url = "";
    std::cout << "\nRoute default constructor called" << std::endl;
    std::cout << "path: " << this->path << std::endl;
    std::cout << "allowed_methods: ";
    for (const auto& method : this->allowed_methods) {
        std::cout << method << " ";
    }
    std::cout << std::endl;
    std::cout << "root_dir: " << this->root_dir << std::endl;
    std::cout << "index_file: " << this->index_file << std::endl;
    std::cout << "autoindex: " << this->autoindex << std::endl;
    std::cout << "redirect_status: " << this->redirect_status << std::endl;
    std::cout << "redirect_url: " << this->redirect_url << "\n" << std::endl;
}

Route::~Route() {
    std::cout << "Route destructor called" << std::endl;
}