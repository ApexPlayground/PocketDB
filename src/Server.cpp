#include "../include/Server.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

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

    // set up local piv4 address
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
}
