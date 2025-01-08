#include "../headers/config.hpp"
#include "../headers/server.hpp"
#include "../headers/HttpReq.hpp"

int main () {
    //config file
    Config config;
    Route route;
    config.addRoute(route);

    //server
    Server server;
    server.start();
    sleep(10);
    server.stop();
    return 0;
}