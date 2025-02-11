#include "../headers/AllHeaders.hpp"

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage: ./webserver <config_file>" << std::endl;
		return 1;
	}
	ServerManager serverManager;
	if (serverManager.setServers(argv[1]) == 1)
		return 1;
	serverManager.startServers();
	return 0;
}
