#pragma once

#include <string>

class CommandHandler {
  public:
    CommandHandler();
    // Process command from client and return RESP-formatted response.
    std::string processCommand(const std::string& commandLine);
};