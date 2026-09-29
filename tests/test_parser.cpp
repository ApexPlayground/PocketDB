#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> parseRespCommand(const std::string& input);

int main() {
    std::string input = "*2\r\n$4\r\nPING\r\n$4\r\nTEST\r\n";

    std::vector<std::string> result = parseRespCommand(input);

    for (const std::string& token : result) {
        std::cout << token << '\n';
    }

    return 0;
}