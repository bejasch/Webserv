#include "../headers/HttpRes.hpp"

HttpRes::HttpRes() {}
HttpRes::~HttpRes() {}

void HttpRes::handleRequest(HttpReq *httpRequest) {
    // - Response headers:
    if (httpRequest->getMethod() == "GET") {
        if (httpRequest->getTarget() == "/info.html")
        {
            std::cout << "GET request for /info.html" << std::endl;
            protocol = httpRequest->getProtocol();
            status = 200;
            status_message = "OK";
            content_type = "text/html"; //here we should dynamically determine the content type
            //content_length = fileLength(httpRequest->getTarget());
            content_length = 15;
            //body = parseFile(httpRequest->getTarget());
            body = "<h1>Hello world!</h1>";
        }
        else
        {
            protocol = httpRequest->getProtocol();
            status = 200;
            status_message = "OK";
            content_type = "text/html"; //here we should dynamically determine the content type
            //content_length = fileLength("www/error_404.html");
            content_length = 13;
            //body = parseFile("www/error_404.html");
            body = "<h1>Hello STD!</h1>";
        }
    }
    else {
        protocol = httpRequest->getProtocol();
        status = 404;
        status_message = "Not Found";
        content_type = "text/html"; //here we should dynamically determine the content type
        content_length = 9;
        //body = parseFile("www/error_404.html");
		// use body.length() instead of hardcoding the length
        body = "Not found!";
    }
}

// std::string HttpRes::parseFile(const std::string &filename) {

//     return -1;
// }

void HttpRes::writeResponse(int new_socket) {
    // Build the status line
    std::ostringstream response_stream;
    response_stream << protocol << " " << status << " " << status_message << "\n";

    // Add headers
    response_stream << "Content-Type: " << content_type << "\n";
    response_stream << "Content-Length: " << content_length << "\n\n";
    // response_stream << "Connection: keep-alive\n\n"; //without this we still be stuck in the loop (still issues here)

    // Add body
    response_stream << body;

    // Convert the response to a C string
    std::string response_string = response_stream.str();
    const char* response_cstr = response_string.c_str();

    // Write the response to the socket
    write(new_socket, response_cstr, strlen(response_cstr));
}
