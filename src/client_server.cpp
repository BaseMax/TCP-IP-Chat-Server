#include <iostream>
#include <string>
#include "server.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    Server server(port);
    if (!server.start()) {
        std::cerr << "Failed to start the server." << std::endl;
        return 1;
    }

    server.run();

    return 0;
}