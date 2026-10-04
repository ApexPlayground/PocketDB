#include "../include/Database.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

Database& Database::getInstance() {
    static Database instance;
    return instance;
}

// Common Comands
bool Database::flushAll() {
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store.clear();
    list_store.clear();
    hash_store.clear();
    return true;
}

// ---------------------Key/Value Operations---------------------
void Database::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store[key] = value;
}

bool Database::get(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    // purgeExpired();
    auto it = kv_store.find(key);
    if (it != kv_store.end()) {
        value = it->second;
        return true;
    }
    return false;
}

std::vector<std::string> Database::keys() {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<std::string> result;

    for (const auto& pair : kv_store) {
        result.push_back(pair.first);
    }
    for (const auto& pair : list_store) {
        result.push_back(pair.first);
    }
    for (const auto& pair : hash_store) {
        result.push_back(pair.first);
    }

    return result;
}

std::string Database::type(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    purgeExpired();
    if (kv_store.find(key) != kv_store.end())
        return "string";
    if (list_store.find(key) != list_store.end())
        return "list";
    if (hash_store.find(key) != hash_store.end())
        return "hash";
    else
        return "none";
}

bool Database::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    purgeExpired();
    bool erased = false;
    erased |= kv_store.erase(key) > 0;
    erased |= list_store.erase(key) > 0;
    erased |= hash_store.erase(key) > 0;
    return erased;
}

bool Database::expire(const std::string& key, int seconds) {
    std::lock_guard<std::mutex> lock(db_mutex);
    // purgeExpired();
    bool exist = (kv_store.find(key) != kv_store.end()) ||
                 (list_store.find(key) != list_store.end()) ||
                 (hash_store.find(key) != hash_store.end());

    if (!exist) {
        return false;
    }

    expiry_map[key] =
        std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    return true;
}

void Database::purgeExpired() {}

bool Database::rename(const std::string& oldKey, const std::string& newKey) {
    std::lock_guard<std::mutex> lock(db_mutex);

    // purgeExpired();
    bool found = false;

    // find the old key and tranfer it values top the new key
    auto itKv = kv_store.find(oldKey);
    if (itKv != kv_store.end()) {
        kv_store[newKey] = itKv->second;
        kv_store.erase(itKv);
        found = true;
    }

    auto itList = list_store.find(oldKey);
    if (itList != list_store.end()) {
        list_store[newKey] = itList->second;
        list_store.erase(itList);
        found = true;
    }

    auto itHash = hash_store.find(oldKey);
    if (itHash != hash_store.end()) {
        hash_store[newKey] = itHash->second;
        hash_store.erase(itHash);
        found = true;
    }

    auto itExpire = expiry_map.find(oldKey);
    if (itExpire != expiry_map.end()) {
        expiry_map[newKey] = itExpire->second;
        expiry_map.erase(itExpire);
    }

    return found;
}

// ---------------------list Operations---------------------
ssize_t Database::llen(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end()) {
        return it->second.size();
    }

    return 0;
}

std::vector<std::string> Database::lrange(const std::string& key, int start,
                                          int stop) {
    std::lock_guard<std::mutex> lock(db_mutex);

    auto it = list_store.find(key);

    if (it == list_store.end()) {
        return {};
    }

    const auto& list = it->second;
    int size = static_cast<int>(list.size());

    if (start < 0)
        start = size + start;

    if (stop < 0)
        stop = size + stop;

    if (start < 0)
        start = 0;

    if (stop >= size)
        stop = size - 1;

    if (start >= size || start > stop) {
        return {};
    }

    std::vector<std::string> result;

    for (int i = start; i <= stop; i++) {
        result.push_back(list[i]);
    }

    return result;
}

void Database::lpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);

    list_store[key].insert(list_store[key].begin(), value);
}

void Database::rpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);

    list_store[key].push_back(value);
}

bool Database::lpop(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);

    if (it != list_store.end() && !it->second.empty()) {
        value = it->second.front();
        it->second.erase(it->second.begin());
        return true;
    }
    return false;
}

bool Database::rpop(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);

    if (it != list_store.end() && !it->second.empty()) {
        value = it->second.back();
        it->second.pop_back();
        return true;
    }
    return false;
}

int Database::lrem(const std::string& key, int count,
                   const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);

    auto it = list_store.find(key);

    if (it == list_store.end())
        return 0;

    auto& list = it->second;
    int removed = 0;

    // Remove all matches
    if (count == 0) {
        for (auto iter = list.begin(); iter != list.end();) {
            if (*iter == value) {
                iter = list.erase(iter);
                removed++;
            } else {
                iter++;
            }
        }
    }

    // Remove from left to right
    else if (count > 0) {
        for (auto iter = list.begin(); iter != list.end() && removed < count;) {

            if (*iter == value) {
                iter = list.erase(iter);
                removed++;
            } else {
                iter++;
            }
        }
    }

    // Remove from right to left
    else {
        for (int i = static_cast<int>(list.size()) - 1;
             i >= 0 && removed < -count; i--) {

            if (list[i] == value) {
                list.erase(list.begin() + i);
                ++removed;
            }
        }
    }

    return removed;
}

bool Database::lindex(const std::string& key, int index, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it == list_store.end()) {
        return false;
    }

    const auto& lst = it->second;
    if (index < 0) {
        index = lst.size() + index;
    }

    if (index < 0 || index >= static_cast<int>(lst.size())) {
        return false;
    }

    value = lst[index];
    return true;
}

bool Database::lset(const std::string& key, int index,
                    const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it == list_store.end()) {
        return false;
    }

    auto& lst = it->second;
    if (index < 0) {
        index = lst.size() + index;
    }

    if (index < 0 || index >= static_cast<int>(lst.size())) {
        return false;
    }

    lst[index] = value;
    return true;
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