
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
        std::map<int, std::string> error_pages;
        std::vector<std::string> allowed_methods;
        std::vector<Route *> routes;
        CGI        *cgi;

    public:
        Config();
        ~Config();        

        //setter
        void setPort(int port);
        void setName(std::string name);
        void setRootDir(std::string root_dir);
        void setMaxBodySize(int max_body_size);
        void setDefaultFile(std::string default_file);
        void setErrorPage(int error_status, std::string error_page);
        void setAllowedMethods(const std::vector<std::string> &allowed_methods);
        void addRoute(Route *route);
        void addCGI(CGI *cgi);

        //getter
        int getPort() const {return this->server_port;};
        std::string getName() const {return this->server_name;};
        std::string getRootDir() const {return this->root_dir;};
        int getMaxBodySize() const {return this->max_body_size;};
        std::string getDefaultFile() const {return this->default_file;};
        std::map<int, std::string> getErrorPages() const {return this->error_pages;};
        CGI		*getCGI() const { return this->cgi; };


		Route	*getRouteForTarget(std::string &target);

        //utils
        int initialisedCheck();
        void printConfig();
        void freeConfig();
};

#endif
