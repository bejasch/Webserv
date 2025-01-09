#include <iostream>
#include "../headers/HttpReq.hpp"

void testHttpRequest(const std::string &requestData) {
    HttpReq httpRequest;

    std::cout << "\n\t---> NEW raw HTTP Request:\n" << requestData << "\n^^^^^^^^^^^^^\n";

    int parse_status = httpRequest.parse(requestData);

    // Print parsed results
    std::cout << "\t#### Parsed Request:\n";
    httpRequest.print();
	std::cout << "Parse-status: " << parse_status << "\n";
    std::cout << "\n\t###---\n";

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

	std::string test4 =
		"GET /shared-assets/images/diagrams/http/messages/response-headers.svg HTTP/1.1\r\n"
		"Host: mdn.github.io\r\n"
		"User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:134.0) Gecko/20100101 Firefox/134.0\r\n"
		"Accept: image/avif,image/webp,image/png,image/svg+xml,image/*;q=0.8,*/*;q=0.5\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n"
		"Accept-Encoding: gzip, deflate, br, zstd\r\n"
		"Referer: https://developer.mozilla.org/\r\n";
	
	testHttpRequest(test4);

    return (0);
}
