
#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "AllHeaders.hpp"

class Config {
    private:
        int         server_port;
        std::string server_name;
        std::string root_dir;
        int         max_body_size;
        std::string default_file;
        std::string error_file;
        int         error_status;

        std::vector<Route> routes;

    public:
        Config();
        ~Config();
        void parseConfigFile(const std::string& config_file);
        void parseServer(std::ifstream& file);
        void parseRoute(std::ifstream& file);
        void addRoute(const Route& route);
        void printConfig(const Config& config);
};

#endif
