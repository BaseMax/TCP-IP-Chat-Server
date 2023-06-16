#include "client.h"

Client::Client(int socket, const std::string& address, int port) : socket(socket), address(address), port(port) {}

int Client::getSocket() const {
    return socket;
}

std::string Client::getAddress() const {
    return address;
}

int Client::getPort() const {
    return port;
}