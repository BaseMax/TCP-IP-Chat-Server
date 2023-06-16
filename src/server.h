#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <fstream>
#include "client.h"

const int BACKLOG = 10;
const int MESSAGE_SIZE = 4096;

class Server {
public:
    explicit Server(int port);
    ~Server();

    bool start();
    void run();

private:
    void disconnectClient(int clientSocket);
    void handleMessage(int senderSocket, const std::string& message);
    void listClients(int clientSocket);
    void kickClient(int senderSocket, int clientId);
    int getClientIndex(int clientSocket) const;
    void logMessage(const std::string& message);

    int port;
    bool running;
    int maxClients;
    int serverSocket;
    std::vector<Client> clients;
    fd_set masterSet;
    std::ofstream logFile;
};

#endif  // SERVER_H
