
#pragma once
#ifndef ROUTE_HPP
#define ROUTE_HPP

#include "AllHeaders.hpp"

class Route {
    private:
        std::string                 path;
        std::vector<std::string>    allowed_methods; //{"GET", "POST", "DELETE", "PUT"};
        std::string                 root_dir;
        std::string                 index_file;
        bool                        autoindex;
        int                         redirect_status;
        std::string                 redirect_url;

    public:
        Route();
        ~Route();
        
        //setter and getter functions
        void setPath(const std::string &path);
        void setAllowedMethods(const std::vector<std::string> &allowed_methods);
        void setRootDir(const std::string &root_dir);
        void setIndexFile(const std::string &index_file);
        void setAutoindex(std::string autoindex);
        void setRedirectStatus(int redirect_status);
        void setRedirectUrl(const std::string &redirect_url);

        //utils
        void printRoute(const Route &route);
};

#endif
