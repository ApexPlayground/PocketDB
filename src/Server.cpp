#include "../include/Server.hpp"
#include "../include/CommandHandler.hpp"

#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

static Server* globalServer = nullptr;

Server::Server(int port) : port(port), server_socket(-1), running(true) {
    globalServer = this;
}

void Server::shutdown() {
    running = false;

    if (server_socket != -1) {
        close(server_socket);
    }

    std::cout << "Server Shutdown Completed";
}

void Server::run() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        std::cerr << "Error Creating Server Socket\n";
        return;
    }

    // customizing socket option
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // set up local IPv4 address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // bind socket to local address and port
    if (bind(server_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) <
        0) {

        std::cerr << "Error Binding Server Socket\n";
        return;
    }
    // Start listening for incoming client connections
    if (listen(server_socket, 10) < 0) {
        std::cerr << "Error Listening On Server Socket\n";
        return;
    }

    std::cout << "PocketDB Server Listening On Port " << port << '\n';

    std::vector<std::thread> threads;
    CommandHandler cmdHandler;

    while (running) {
        // Wait for a client to connect
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            if (running) {
                std::cerr << "Error Accepting Client Connection";
            }
            break;
        }
        // Handle each client in its own thread
        threads.emplace_back([client_socket, &cmdHandler]() {
            char buffer[1024];
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                // Read data sent by the client
                int bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

                if (bytes <= 0) {
                    break;
                }
                // Process the request and send the response
                std::string request(buffer, bytes);
                std::string response = cmdHandler.processCommand(request);
                send(client_socket, response.c_str(), response.size(), 0);
            }
            close(client_socket);
        });
    }
    // Wait for all client threads to finish
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}
