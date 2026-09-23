#include "../include/Server.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    int port = 6379; // default port

    if (argc >= 2) {
        port = std::stoi(argv[1]); // user specific port
    }

    Server server(port);
    return 0;
}