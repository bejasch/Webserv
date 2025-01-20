#pragma once
#ifndef ROUTE_HPP
#define ROUTE_HPP

#include "AllHeaders.hpp"

class CGI {
private:
    std::map<std::string, std::string>  env;
    std::string                         cgiPath;
    std::string                         cgiName;
    std::string                         cgiArgs;
    int                                 pid;

public:
    CGI();
    ~CGI();
    void executeCGI(HttpReq &httpRequest, Server &server);
};