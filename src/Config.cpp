#include "../headers/AllHeaders.hpp"

Config::Config() {
    std::cout << "Config default constructor called" << std::endl;
}

Config::~Config() {
    std::cout << "Config destructor called" << std::endl;
}

void Config::printConfig(const Config& config) {
    std::cout << "server_port: " <<  config.server_port << std::endl;
    std::cout << "server_name: " << config.server_name << std::endl;
    std::cout << "root_dir: " << config.root_dir << std::endl;
    std::cout << "max_body_size: " << config.max_body_size << std::endl;
    std::cout << "default_file: " << config.default_file << std::endl;
    std::cout << "error_file: " << config.error_file << std::endl;
    std::cout << "error_status: " << config.error_status << std::endl;
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

void Config::setErrorFile(std::string error_file) {
    this->error_file = error_file;
}

void Config::setErrorStatus(int error_status) {
    this->error_status = error_status;
}

void Config::addRoute(Route *route) {
    this->routes.push_back(*route);
}
