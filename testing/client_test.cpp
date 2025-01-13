#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

void sendData(int sockfd, const std::string &request) {
    usleep(500000);
    if (send(sockfd, request.c_str(), request.size(), 0) == -1) {
        perror("Failed to send data");
    }
}

void testSimpleRequest(int sockfd) {
    usleep(500000);
    std::cout << "\n--- Testing Simple HTTP Request ---\n";
    std::string request = "GET / HTTP/1.1\r\n"
                          "Host: localhost\r\n"
                          "Connection: close\r\n\r\n";
    sendData(sockfd, request);
}

void testChunkedRequest(int sockfd) {
    std::cout << "\n--- Testing Chunked HTTP Request ---\n";
    std::string initialHeaders = "POST /chunked HTTP/1.1\r\n"
                                 "Host: localhost\r\n"
                                 "Transfer-Encoding: chunked\r\n\r\n";
    sendData(sockfd, initialHeaders);

    std::string chunk1 = "4\r\nWiki\r\n";
    sendData(sockfd, chunk1);

    std::string chunk2 = "5\r\npedia\r\n";
    sendData(sockfd, chunk2);

    std::string finalChunk = "0\r\n\r\n";
    sendData(sockfd, finalChunk);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        return 1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return 1;
    }

    // Run tests
    // testSimpleRequest(sockfd);
    testChunkedRequest(sockfd);

    // Close the socket
    close(sockfd);
    return 0;
}
