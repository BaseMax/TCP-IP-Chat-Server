#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "server.h"

Server::Server(int port) : port(port), running(false), maxClients(0), nextClientId(1), logFile("chat_log.txt") {
    FD_ZERO(&masterSet);
}

Server::~Server() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

bool Server::start() {
    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        return false;
    }

    // Set socket options
    int reuseAddr = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(int)) == -1) {
        perror("Error setting socket options");
        return false;
    }

    // Bind the socket to the port
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    memset(&(serverAddress.sin_zero), '\0', 8);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(sockaddr)) == -1) {
        perror("Error binding socket");
        return false;
    }

    // Start listening
    if (listen(serverSocket, BACKLOG) == -1) {
        perror("Error listening on socket");
        return false;
    }

    FD_SET(serverSocket, &masterSet);
    maxClients = serverSocket;
    std::cout << "Server started on port " << port << std::endl;
    return true;
}

void Server::run() {
    running = true;

    while (running) {
        fd_set readSet = masterSet;
        if (select(maxClients + 1, &readSet, nullptr, nullptr, nullptr) == -1) {
            perror("Error in select");
            break;
        }

        for (int i = 0; i <= maxClients; ++i) {
            if (FD_ISSET(i, &readSet)) {
                if (i == serverSocket) {
                    // New connection
                    sockaddr_in clientAddress{};
                    socklen_t addrLength = sizeof(clientAddress);
                    int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &addrLength);
                    if (clientSocket == -1) {
                        perror("Error accepting connection");
                    } else {
                        // Add the new client to the list
                        FD_SET(clientSocket, &masterSet);
                        maxClients = std::max(maxClients, clientSocket);
                        clients.push_back(Client(clientSocket, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port)));

                        // Log the connection
                        std::stringstream logMsg;
                        logMsg << "Client connected: " << clientSocket << " (" << clients.back().getAddress() << ":" << clients.back().getPort() << ")";
                        logMessage(logMsg.str());
                        std::cout << logMsg.str() << std::endl;
                    }
                } else {
                    // Handle client data
                    char buffer[MESSAGE_SIZE];
                    int bytesRead = recv(i, buffer, MESSAGE_SIZE - 1, 0);
                    if (bytesRead <= 0) {
                        if (bytesRead == 0) {
                            // Connection closed
                            disconnectClient(i);
                        } else {
                            perror("Error receiving data");
                        }
                    } else {
                        buffer[bytesRead] = '\0';
                        handleMessage(i, buffer);
                    }
                }
            }
        }
    }

    // Clean up
    for (int i = 0; i <= maxClients; ++i) {
        if (FD_ISSET(i, &masterSet)) {
            close(i);
        }
    }
}
void Server::disconnectClient(int clientSocket) {
    auto it = std::find_if(clients.begin(), clients.end(), [clientSocket](const Client& client) {
        return client.getSocket() == clientSocket;
    });

    if (it != clients.end()) {
        std::stringstream logMsg;
        logMsg << "Client disconnected: " << clientSocket << " (" << it->getAddress() << ":" << it->getPort() << ")";
        logMessage(logMsg.str());
        std::cout << logMsg.str() << std::endl;

        close(clientSocket);
        FD_CLR(clientSocket, &masterSet);
        clients.erase(it);
    }
}

void Server::handleMessage(int senderSocket, const std::string& message) {
    if (message.substr(0, 5) == "/list") {
        listClients(senderSocket);
    } else if (message.substr(0, 5) == "/kick") {
        int clientId = std::stoi(message.substr(6));
        kickClient(senderSocket, clientId);
    } else if (!message.empty() && message != "\n") {
        // Broadcast the message to all connected clients
        for (const Client& client : clients) {
            if (client.getSocket() != senderSocket) {
                send(client.getSocket(), message.c_str(), message.length(), 0);
            }
        }

        // Log the message
        std::stringstream logMsg;
        logMsg << "Message from client " << senderSocket << " (" << clients[getClientIndex(senderSocket)].getAddress() << ":" << clients[getClientIndex(senderSocket)].getPort() << "): " << message;
        logMessage(logMsg.str());
        std::cout << logMsg.str() << std::endl;
    }
}

void Server::listClients(int clientSocket) {
    std::stringstream response;
    response << "Connected clients:" << std::endl;

    for (const Client& client : clients) {
        response << client.getSocket() << ". " << client.getAddress() << ":" << client.getPort() << std::endl;
    }

    send(clientSocket, response.str().c_str(), response.str().length(), 0);
}

void Server::kickClient(int senderSocket, int clientId) {
    // if (senderSocket != serverSocket) {
    //     send(senderSocket, "You are not authorized to kick clients.", 37, 0);
    //     return;
    // }

    auto it = std::find_if(clients.begin(), clients.end(), [clientId](const Client& client) {
        return client.getSocket() == clientId;
    });

    if (it != clients.end()) {
        disconnectClient(clientId);
        std::string response = "Client " + std::to_string(clientId) + " has been kicked.";
        send(senderSocket, response.c_str(), response.length(), 0);
    } else {
        std::string response = "Client " + std::to_string(clientId) + " not found.";
        send(senderSocket, response.c_str(), response.length(), 0);
    }
}

int Server::getClientIndex(int clientSocket) const {
    auto it = std::find_if(clients.begin(), clients.end(), [clientSocket](const Client& client) {
        return client.getSocket() == clientSocket;
    });

    if (it != clients.end()) {
        return std::distance(clients.begin(), it);
    }

    return -1;
}

void Server::logMessage(const std::string& message) {
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.flush();
    }
}