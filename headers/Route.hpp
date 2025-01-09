
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
};

#endif
