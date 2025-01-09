#include "../headers/AllHeaders.hpp"

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

void HttpRes::writeResponse(int client_fd) {
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
    write(client_fd, response_cstr, strlen(response_cstr));
}


/*
// ---------- OK - response

HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
Content-Length: 55743
Connection: keep-alive
Cache-Control: s-maxage=300, public, max-age=0
Content-Language: en-US
Date: Thu, 06 Dec 2018 17:37:18 GMT
ETag: "2e77ad1dc6ab0b53a2996dfd4653c1c3"
Server: meinheld/0.6.1
Strict-Transport-Security: max-age=63072000
X-Content-Type-Options: nosniff
X-Frame-Options: DENY
X-XSS-Protection: 1; mode=block
Vary: Accept-Encoding,Cookie
Age: 7

<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>A simple webpage</title>
</head>
<body>
  <h1>Simple HTML webpage</h1>
  <p>Hello, world!</p>
</body>
</html>



// ---------- MOVE - response

HTTP/1.1 301 Moved Permanently
Server: Apache/2.4.37 (Red Hat)
Content-Type: text/html; charset=utf-8
Date: Thu, 06 Dec 2018 17:33:08 GMT
Location: https://developer.mozilla.org/ (this is the new link to the resource; it is expected that the user-agent will fetch it)
Keep-Alive: timeout=15, max=98
Accept-Ranges: bytes
Via: Moz-Cache-zlb05
Connection: Keep-Alive
Content-Length: 325 (the content contains a default page to display if the user-agent is not able to follow the link)

<!doctype html>… (contains a site-customized page helping the user to find the missing resource)

*/
