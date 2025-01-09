#include "../headers/AllHeaders.hpp"

Config::Config() {
    this->server_port = 8080;
    this->server_name = "localhost";
    this->root_dir = "data/";
    this->max_body_size = 200000000;
    this->default_file = "www/index.html";
    this->error_file = "www/error_404.html";
    this->error_status = 404;
    // std::cout << "\nConfig default constructor called" << std::endl;
    // std::cout << "server_port: " << this->server_port << std::endl;
    // std::cout << "server_name: " << this->server_name << std::endl;
    // std::cout << "root_dir: " << this->root_dir << std::endl;
    // std::cout << "max_body_size: " << this->max_body_size << std::endl;
    // std::cout << "default_file: " << this->default_file << std::endl;
    // std::cout << "error_file: " << this->error_file << std::endl;
    // std::cout << "error_status: " << this->error_status << "\n" << std::endl;
}

Config::~Config() {
    std::cout << "Config destructor called" << std::endl;
}

void Config::addRoute(const Route& route) {
    routes.push_back(route);
    std::cout << "Route added" << std::endl;
}
