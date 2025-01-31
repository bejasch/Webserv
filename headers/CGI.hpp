#pragma once
#ifndef CGI_HPP
#define CGI_HPP

#include "AllHeaders.hpp"

class CGI {
private:
    int                                 pid;
    Route                               *route;
    std::map<std::string, std::string>  env;

public:
    CGI(Route *route);
    ~CGI();
    void setAllEnv(HttpReq &httpRequest, Server &server);
    std::string executeCGI(HttpReq &httpRequest, Server &server, std::string &args);
    void printCGI();
};

#endif