#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <string>
#include <iostream>
#include "route.hpp"

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
        void addRoute(const Route& route);
        //setter and getter functions
};

#endif