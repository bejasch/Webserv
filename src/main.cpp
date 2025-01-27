#include "../headers/AllHeaders.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./webserver <config_file>" << std::endl;
        return 1;
    }
    ServerManager serverManager;
    serverManager.setServers(argv[1]);
    serverManager.startServers();
    return 0;
}
