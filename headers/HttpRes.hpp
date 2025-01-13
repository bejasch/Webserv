
#pragma once
#ifndef HTTPRES_HPP
#define HTTPRES_HPP

#include "AllHeaders.hpp"

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
        void handleRequest(HttpReq &httpRequest);
        void writeResponse(int client_fd);

    private:
        std::string parseFile(const std::string &filename);
        std::string determineContentType(const std::string &filename);
};

#endif
