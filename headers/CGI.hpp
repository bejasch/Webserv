#pragma once
#ifndef CGI_HPP
#define CGI_HPP

#include "AllHeaders.hpp"

class CGI {
private:
    pid_t                               pid;
    std::map<std::string, std::string>  env;

public:
    CGI();
    ~CGI();
    void setAllEnv(HttpReq &httpRequest);
    std::string executeCGI_GET(HttpReq &httpRequest);
    std::string executeCGI_POST(HttpReq &httpRequest, const std::map<std::string, std::string> &formData);
    void printCGI();
};

#endif