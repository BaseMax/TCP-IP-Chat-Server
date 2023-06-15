#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <ctime>
#include <fstream>
#include <vector>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

struct Client {
    int socket;
    std::string username;
};

std::vector<Client> clients;

void broadcastMessage(const std::string& message, int senderSocket) {
    for (const auto& client : clients) {
        if (client.socket != senderSocket) {
            send(client.socket, message.c_str(), message.size(), 0);
        }
    }
}

void sendClientList(int socket) {
    std::string clientList = " Connected Clients \n";
    for (const auto& client : clients) {
        clientList += client.username + "\n";
    }
    send(socket, clientList.c_str(), clientList.size(), 0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
        return 1;
    }

    int serverSocket, maxSocket, activity, newSocket;
    struct sockaddr_in serverAddress;
    fd_set readFds;
    char buffer[BUFFER_SIZE] = {0};

    // Create server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // Set up server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(std::atoi(argv[1]));

    // Bind server socket to the specified port
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        std::cerr << "Failed to listen" << std::endl;
        return 1;
    }

    std::cout << "Server started. Listening on port " << argv[1] << std::endl;

    // Accept incoming connections and handle data
    while (true) {
        FD_ZERO(&readFds);
        FD_SET(serverSocket, &readFds);
        maxSocket = serverSocket;

        for (const auto& client : clients) {
            int clientSocket = client.socket;
            FD_SET(clientSocket, &readFds);
            if (clientSocket > maxSocket) {
                maxSocket = clientSocket;
            }
        }

        activity = select(maxSocket + 1, &readFds, nullptr, nullptr, nullptr);
        if (activity < 0 && errno != EINTR) {
            std::cerr << "Failed to select" << std::endl;
            break;
        }

        if (FD_ISSET(serverSocket, &readFds)) {
            newSocket = accept(serverSocket, nullptr, nullptr);
            if (newSocket < 0) {
                std::cerr << "Failed to accept" << std::endl;
                break;
            }

            std::cout << "New connection, socket fd: " << newSocket << std::endl;

            // Add new client to the vector
            clients.push_back({newSocket, ""});
        }

        for (auto it = clients.begin(); it != clients.end(); ++it) {
            int clientSocket = it->socket;
            if (FD_ISSET(clientSocket, &readFds)) {
                // Handle data from client
                int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead <= 0) {
                    // Client disconnected
                    getpeername(clientSocket, (struct sockaddr*)&serverAddress, (socklen_t*)&serverAddress);
                    std::cout << "Client disconnected, socket fd: " << clientSocket << ", IP: " << inet_ntoa(serverAddress.sin_addr) << ", Port: " << ntohs(serverAddress.sin_port) << std::endl;

                    // Close the socket and remove from the vector
                    close(clientSocket);
                    clients.erase(it);

                    // Broadcast client list to other clients
                    std::string clientListUpdated = " Client Disconnected \n";
                    broadcastMessage(clientListUpdated, clientSocket);
                    break;
                } else {
                    // Process received message
                    std::string message(buffer);
                    if (message.find("/list") == 0) {
                        // Handle /list command
                        std::cout << "List command received" << std::endl;

                        // Send client list to the requesting client
                        sendClientList(clientSocket);
                    } else if (message.find("/kick") == 0) {
                        // Handle /kick command
                        std::cout << "Kick command received" << std::endl;

                        // Extract client ID from the message
                        std::string clientIdStr = message.substr(message.find(' ') + 1);
                        int clientId = std::stoi(clientIdStr);

                        // Disconnect the specified client
                        if (clientId >= 0 && clientId < clients.size()) {
                            int kickSocket = clients[clientId].socket;
                            std::cout << "Disconnecting client with ID: " << clientId << std::endl;
                            close(kickSocket);
                            clients.erase(clients.begin() + clientId);

                            // Broadcast client list to other clients
                            std::string clientListUpdated = " Client Disconnected \n";
                            broadcastMessage(clientListUpdated, kickSocket);
                        } else {
                            std::cout << "Invalid client ID" << std::endl;
                            send(clientSocket, "Invalid client ID\n", 18, 0);
                        }
                    } else {
                        // Broadcast message to other clients
                        std::string sender;
                        for (const auto& client : clients) {
                            if (client.socket == clientSocket) {
                                sender = client.username;
                                break;
                            }
                        }
                        std::string formattedMessage = "[" + sender + "]: " + message;

                        std::cout << "Received message from " << sender << ": " << message << std::endl;

                        // Log message to file
                        std::ofstream logFile("chat_log.txt", std::ios_base::app);
                        if (logFile.is_open()) {
                            std::time_t now = std::time(nullptr);
                            logFile << "[" << std::asctime(std::localtime(&now)) << "] " << sender << ": " << message << std::endl;
                            logFile.close();
                        }

                        // Broadcast the message to other clients
                        broadcastMessage(formattedMessage, clientSocket);
                    }
                }
            }
        }
    }

    // Close server socket
    close(serverSocket);

    return 0;
}