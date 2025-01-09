#include "../headers/AllHeaders.hpp"

int main (int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./webserver <config_file>" << std::endl;
        return 1;
    }
    //config file
    Config config;
    Route route;
    config.addRoute(route);

    //server
    // Server server;
    // server.start();
    return 0;
}
