#include "../headers/config.hpp"
#include "../headers/server.hpp"

int main () {
    Config config;
    Route route;
    config.addRoute(route);

    Server server;
    server.start();
    sleep(10);
    server.stop();
    return 0;
}