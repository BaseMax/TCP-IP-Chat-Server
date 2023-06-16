#ifndef CLIENT_H
#define CLIENT_H

#include <string>

class Client {
public:
    Client(int socket, const std::string& address, int port);

    int getSocket() const;
    std::string getAddress() const;
    int getPort() const;

private:
    int socket;
    std::string address;
    int port;
};

#endif  // CLIENT_H