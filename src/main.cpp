#include "../include/Server.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char* argv[]) {
    int port = 6379; // default port

    if (argc >= 2) {
        port = std::stoi(argv[1]); // user specific port
    }

    Server server(port);

    // Backgroud persistence: dump the database every 5 min (5 * 60 = 300
    // secounds)
    std::thread persistenceThread([]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(300));
            // dump the database
        }
    });
    persistenceThread.detach();

    server.run();
    return 0;
}