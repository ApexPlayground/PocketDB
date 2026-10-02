#include "../include/CommandHandler.hpp"
#include "../include/Database.hpp"

#include <cctype>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// RESP parser
/**
 *2\r\n$4\r\nPING\r\n$4\r\nTEST\r\n

 this means:
 *2 = array has 2 elements
 $4 = next string has 4 characters

 after parsing we have ping and test
 PING
 TEST

 */

std::vector<std::string> parseRespCommand(const std::string& input) {
    std::vector<std::string> tokens;

    if (input.empty()) {
        return tokens;
    }

    // split by whitespace if it dosnt start with '*'
    if (input[0] != '*') {
        std::istringstream iss(input);
        std::string token;

        while (iss >> token) {
            tokens.push_back(token);
        }

        return tokens;
    }

    size_t pos = 0;

    // parsing RESP array so expecting '*'
    if (input[pos] != '*') {
        return tokens;
    }

    pos++; // move past '*'

    // Find the end of the array-length line
    size_t lineEnd = input.find("\r\n", pos);

    if (lineEnd == std::string::npos) {
        return tokens;
    }

    // Read number of elements, e.g. "*2\r\n" is 2
    std::string countText = input.substr(pos, lineEnd - pos);
    int numElements = std::stoi(countText);

    // Move to the first RESP element
    pos = lineEnd + 2;

    for (int i = 0; i < numElements; i++) {
        if (pos >= input.size() || input[pos] != '$') {
            break;
        }
        pos++;

        lineEnd = input.find("\r\n", pos);
        if (lineEnd == std::string::npos) {
            break;
        }

        int len = std::stoi(input.substr(pos, lineEnd - pos));
        pos = lineEnd + 2;

        if (pos + len > input.size()) {
            break;
        }

        std::string token = input.substr(pos, len);
        tokens.push_back(token);
        pos += len + 2;
    }
    return tokens;
}

CommandHandler::CommandHandler() {}

std::string CommandHandler::processCommand(const std::string& commandLine) {
    auto tokens = parseRespCommand(commandLine);

    if (tokens.empty()) {
        return "-Error: Empty command\r\n";
    }

    // std::cout << commandLine << "\n";
    // for (auto& t : tokens) {
    //     std::cout << t << '\n';
    // }

    std::string cmd = tokens[0];

    for (char& c : cmd) {
        c = std::toupper(c);
    }

    std::ostringstream response;

    // setup & connect to DB
    Database& db = Database::getInstance();

    if (cmd == "PING") {
        response << "+PONG\r\n";
    } else if (cmd == "ECHO") {
        if (tokens.size() < 2) {
            response << "-Error: ECHO requires a message\r\n";
        } else {
            std::string message;

            for (size_t i = 1; i < tokens.size(); ++i) {
                if (i > 1) {
                    message += " ";
                }

                message += tokens[i];
            }

            response << "+" << message << "\r\n";
        }
    } else if (cmd == "FLUSHALL") {
        db.flushAll();
        response << "+OK\r\n";
    } else if (cmd == "SET") {

        if (tokens.size() < 3) {
            response << "-Error: SET requires key and values\r\n";
        } else {
            db.set(tokens[1], tokens[2]);
            response << "+OK\r\n";
        }

    } else if (cmd == "GET") {
        if (tokens.size() < 2) {
            response << "-Error: GET requires key\r\n";
        } else {
            std::string value;

            if (db.get(tokens[1], value)) {
                response << "$" << value.size() << "\r\n" << value << "\r\n";
            } else {
                response << "$-1\r\n";
            }
        }

    } else if (cmd == "KEYS") {
        std::vector<std::string> allKeys = db.keys();
        response << "*" << allKeys.size() << "\r\n";
        for (const auto& key : allKeys) {
            response << "$" << key.size() << "\r\n" << key << "\r\n";
        }
    } else if (cmd == "TYPE") {
        if (tokens.size() < 2) {
            response << "-Error: TYPE requires key\r\n";
        } else {
            response << "+" << db.type(tokens[1]) << "\r\n";
        }
    } else if (cmd == "DEL" || cmd == "UNLINK") {
        if (tokens.size() < 2) {
            response << "-Error: " << cmd << " requires key\r\n";
        } else {
            bool res = db.del(tokens[1]);
            response << ":" << (res ? 1 : 0) << "\r\n";
        }
    } else if (cmd == "EXPIRE") {
        if (tokens.size() < 3) {
            response << "-Error: EXPIRE requires key and time in seconds\r\n";
        } else {
            if (db.expire(tokens[1], std::stoi(tokens[2]))) {
                response << "+OK\r\n";
            }
        }

    } else if (cmd == "RENAME") {
        if (tokens.size() < 3) {
            response
                << "-Error: RENAME requres old key name and new key name\r\n";
        } else {
            if (db.rename(tokens[1], tokens[2])) {
                response << "+OK\r\n";
            }
        }
    } else {
        response << "-Error: Unknown command\r\n";
    }

    return response.str();
}
