#include <iostream>
#include "HttpReq.hpp"

void testHttpRequest(const std::string &requestData) {
    HttpReq httpRequest;

    std::cout << "\n\t###--- Testing HTTP Request:\n" << requestData << "\n";

    int parse_status = httpRequest.parse(requestData);

    // Print parsed results
    std::cout << "Parsed Request:\n";
    httpRequest.print();
    std::cout << "\n";

    // Access individual parts
    std::cout << "Method: ->" << httpRequest.getMethod() << "<-\n";
    std::cout << "Target: ->" << httpRequest.getTarget() << "<-\n";
    std::cout << "Protocol: ->" << httpRequest.getProtocol() << "<-\n";
	std::cout << "Parse-status: ->" << parse_status << "<-\n";
}

int main() {
    // Test Case 1: Valid GET request
    std::string test1 =
        "GET /en-US/docs/Web/HTTP/Messages HTTP/1.1\r\n"
        "Host: developer.mozilla.org\r\n"
        "User-Agent: curl/7.68.0\r\n"
        "Accept: */*\r\n"
        "\r\n";

    testHttpRequest(test1);

    // Test Case 2: Valid POST request with body
    std::string test2 =
        "POST /api/v1/data HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 27\r\n"
        "\r\n"
        "{\"key\":\"value\",\"id\":1}";

    testHttpRequest(test2);

    // Test Case 3: Invalid request line
    std::string test3 =
        "INVALIDLINE\r\n"
        "Host: invalid.example.com\r\n"
        "\r\n";

    testHttpRequest(test3);

    return (0);
}
