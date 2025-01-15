
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

        //setter
        void setPort(int port);
        void setName(std::string name);
        void setRootDir(std::string root_dir);
        void setMaxBodySize(int max_body_size);
        void setDefaultFile(std::string default_file);
        void setErrorFile(std::string error_file);
        void setErrorStatus(int error_status);
        void addRoute(Route *route);

        //getter
        int getPort() const {return this->server_port;};
        std::string getName() const {return this->server_name;};
        std::string getRootDir() const {return this->root_dir;};
        int getMaxBodySize() const {return this->max_body_size;};
        std::string getDefaultFile() const {return this->default_file;};
        std::string getErrorFile() const {return this->error_file;};
        int getErrorStatus() const {return this->error_status;};

        //utils
        void printConfig(const Config& config);
};

#endif
