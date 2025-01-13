#include "../headers/AllHeaders.hpp"

Config::Config() {
    this->server_port = 8080;
    this->server_name = "localhost";
    this->root_dir = "data/";
    this->max_body_size = 200000000;
    this->default_file = "www/index.html";
    this->error_file = "www/error_404.html";
    this->error_status = 404;
    std::cout << "\nConfig default constructor called" << std::endl;
    std::cout << "server_port: " << this->server_port << std::endl;
    std::cout << "server_name: " << this->server_name << std::endl;
    std::cout << "root_dir: " << this->root_dir << std::endl;
    std::cout << "max_body_size: " << this->max_body_size << std::endl;
    std::cout << "default_file: " << this->default_file << std::endl;
    std::cout << "error_file: " << this->error_file << std::endl;
    std::cout << "error_status: " << this->error_status << "\n" << std::endl;
}

int Config::parseConfigFile(const std::string& config_file) {
    std::ifstream file(config_file.c_str());
    if (!file.is_open()) {
        std::cerr << "Error: could not open config file" << std::endl;
        return 1;
    }
    // parseServer(file);
    // parseRoute(file);
    file.close();
    return 0;
}

Config::~Config() {
    std::cout << "Config destructor called" << std::endl;
}

// int Config::parseConfigFile(std::ifstream& file) {
//     std::string line;
//     while (std::getline(file, line)) {
//         if (line.find("server") != std::string::npos) {
//             while (std::getline(file, line)) {
//                 if (line.find("}") != std::string::npos) {
//                     break;
//                 }
//                 if (line.find("server_port") != std::string::npos) {
//                     this->server_port = std::stoi(line.substr(line.find(" ") + 1));
//                 }
//                 else if (line.find("server_name") != std::string::npos) {
//                     this->server_name = line.substr(line.find(" ") + 1);
//                 }
//                 else if (line.find("root_dir") != std::string::npos) {
//                     this->root_dir = line.substr(line.find(" ") + 1);
//                 }
//                 else if (line.find("max_body_size") != std::string::npos) {
//                     this->max_body_size = std::stoi(line.substr(line.find(" ") + 1));
//                 }
//                 else if (line.find("default_file") != std::string::npos) {
//                     this->default_file = line.substr(line.find(" ") + 1);
//                 }
//                 else if (line.find("error_file") != std::string::npos) {
//                     this->error_file = line.substr(line.find(" ") + 1);
//                 }
//                 else if (line.find("error_status") != std::string::npos) {
//                     this->error_status = std::stoi(line.substr(line.find(" ") + 1));
//                 }
//             }
//         }
//     }
// }

void Config::printConfig(const Config& config) {
    std::cout << "server_port: " <<  config.server_port << std::endl;
    std::cout << "server_name: " << config.server_name << std::endl;
    std::cout << "root_dir: " << config.root_dir << std::endl;
    std::cout << "max_body_size: " << config.max_body_size << std::endl;
    std::cout << "default_file: " << config.default_file << std::endl;
    std::cout << "error_file: " << config.error_file << std::endl;
    std::cout << "error_status: " << config.error_status << std::endl;
}

int Config::getPort() const {
    return this->server_port;
}
