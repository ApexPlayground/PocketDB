#include "../include/Database.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

Database& Database::getInstance() {
    static Database instance;
    return instance;
}

bool Database::dump(const std::string& filename) {
    std::lock_guard<std::mutex> lock(db_mutex);

    std::ofstream ofs(filename);

    if (!ofs) {
        return false;
    }

    // Save string key-value pairs
    for (const auto& kv : kv_store) {
        ofs << "K " << std::quoted(kv.first) << " " << std::quoted(kv.second)
            << "\n";
    }

    // Save list keys and their items
    for (const auto& kv : list_store) {
        ofs << "L " << std::quoted(kv.first);

        for (const auto& item : kv.second) {
            ofs << " " << std::quoted(item);
        }

        ofs << "\n";
    }

    // Save hash keys and their field-value pairs
    for (const auto& kv : hash_store) {
        ofs << "H " << std::quoted(kv.first);

        for (const auto& field_val : kv.second) {
            ofs << " " << std::quoted(field_val.first) << " "
                << std::quoted(field_val.second);
        }

        ofs << "\n";
    }

    return true;
}

bool Database::load(const std::string& filename) {
    std::lock_guard<std::mutex> lock(db_mutex);

    std::ifstream ifs(filename);

    if (!ifs) {
        return false;
    }

    std::string line;

    while (std::getline(ifs, line)) {
        std::istringstream iss(line);

        char type;
        iss >> type;

        if (type == 'K') {
            std::string key;
            std::string value;

            iss >> std::quoted(key) >> std::quoted(value);

            kv_store[key] = value;

        } else if (type == 'L') {
            std::string key;
            iss >> std::quoted(key);

            std::vector<std::string> list;
            std::string item;

            while (iss >> std::quoted(item)) {
                list.push_back(item);
            }

            list_store[key] = list;

        } else if (type == 'H') {
            std::string key;
            iss >> std::quoted(key);

            std::unordered_map<std::string, std::string> hash;

            std::string field;
            std::string value;

            while (iss >> std::quoted(field) >> std::quoted(value)) {
                hash[field] = value;
            }

            hash_store[key] = hash;
        }
    }

    return true;
}