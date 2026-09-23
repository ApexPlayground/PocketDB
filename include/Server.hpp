#pragma once

#include <atomic>
#include <string>

class Server {
  public:
    Server(int port);
    void run();
    void shutdown();

  private:
    int port;
    int server_socket;
    std::atomic<bool> running;
};