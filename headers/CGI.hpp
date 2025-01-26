#pragma once
#ifndef CGI_HPP
#define CGI_HPP

#include "AllHeaders.hpp"

class CGI {
private:
    // std::string                         cgiPath; //usr/bin/python
    int                                 pid; //process id
    Route                               *route;
    std::map<std::string, std::string>  env;

public:
    CGI(Route *route);
    ~CGI();
    void setAllEnv(HttpReq &httpRequest, Server &server);
    std::string executeCGI(HttpReq &httpRequest, Server &server);
};

#endif