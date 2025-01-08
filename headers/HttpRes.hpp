#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include "HttpReq.hpp"
#include <unistd.h>
#include <string.h>
#include <cstdlib> //probably illegal, used for malloc
#include <cstdio> //probably illegal, used for sprintf
#include <iostream>
#include <fstream>
#include <sstream>

class HttpRes {
    private:
        // - Response headers:
        std::string	protocol;
        int	status;
        std::string status_message;
        std::string	content_type;
        int	content_length;
        std::string	body;

    public:
        HttpRes();
        ~HttpRes();
        void handleRequest(HttpReq *httpRequest);
        void writeResponse(int new_socket);

    private:
        // int fileLength(const std::string &filename);
        // std::string parseFile(const std::string &filename);
};

#endif