#include "../include/CommandHandler.hpp"
#include <cstddef>
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

    // Read number of elements, e.g. "*2\r\n" -> 2
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
    }
}
