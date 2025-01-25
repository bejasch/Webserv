#pragma once
#ifndef CGI_HPP
#define CGI_HPP

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

#endif