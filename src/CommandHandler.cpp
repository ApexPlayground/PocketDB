#include "../include/CommandHandler.hpp"
#include "../include/Database.hpp"

#include <cctype>
#include <cstddef>
#include <sstream>
#include <string>
#include <sys/types.h>
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

/**

Helper note:
: means integer response
$ means bulk string response
+ means simple string response
- means error
* means array

*/

//--------------------COMMON COMMANDS--------------------

// ping command
static std::string handlePing(const std::vector<std::string>& tokens,
                              Database&) {
    return "+PONG\r\n";
}

// echo command
static std::string handleEcho(const std::vector<std::string>& tokens,
                              Database&) {
    if (tokens.size() < 2) {
        return "-Error: ECHO requires a message\r\n";
    }

    std::string message;

    for (size_t i = 1; i < tokens.size(); ++i) {
        if (i > 1) {
            message += " ";
        }

        message += tokens[i];
    }

    return "+" + message + "\r\n";
}

// flushall command
static std::string handleFlushAll(const std::vector<std::string>& tokens,
                                  Database& db) {
    db.flushAll();
    return "+OK\r\n";
}

//--------------------KEY/VALUE COMMANDS--------------------

// set command
static std::string handleSet(const std::vector<std::string>& tokens,
                             Database& db) {
    if (tokens.size() < 3) {
        return "-Error: SET requires key and values\r\n";
    }

    db.set(tokens[1], tokens[2]);
    return "+OK\r\n";
}

// get command
static std::string handleGet(const std::vector<std::string>& tokens,
                             Database& db) {
    if (tokens.size() < 2) {
        return "-Error: GET requires key\r\n";
    }

    std::string value;

    if (db.get(tokens[1], value)) {
        return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
    }

    return "$-1\r\n";
}

// keys command
static std::string handleKeys(const std::vector<std::string>& tokens,
                              Database& db) {

    std::vector<std::string> allKeys = db.keys();

    std::string response = "*" + std::to_string(allKeys.size()) + "\r\n";

    for (const auto& key : allKeys) {
        response += "$" + std::to_string(key.size()) + "\r\n" + key + "\r\n";
    }

    return response;
}

// type command
static std::string handleType(const std::vector<std::string>& tokens,
                              Database& db) {
    if (tokens.size() < 2) {
        return "-Error: TYPE requires key\r\n";
    }
    return "+" + db.type(tokens[1]) + "\r\n";
}

// delete or unlink command
static std::string handleDeleteOrUnlink(const std::vector<std::string>& tokens,
                                        Database& db) {
    if (tokens.size() < 2) {
        return "-Error: " + tokens[0] + " requires key\r\n";
    }

    bool res = db.del(tokens[1]);

    return ":" + std::to_string(res ? 1 : 0) + "\r\n";
}

// expire command
static std::string handleExpire(const std::vector<std::string>& tokens,
                                Database& db) {
    if (tokens.size() < 3) {
        return "-Error: EXPIRE requires key and time in seconds\r\n";
    }

    try {
        int seconds = std::stoi(tokens[2]);

        if (db.expire(tokens[1], seconds)) {
            return "+OK\r\n";
        }

        return "-Error: Key not found\r\n";

    } catch (const std::exception&) {
        return "-Error: Invalid expiration time\r\n";
    }
}

// rename command
static std::string handleRename(const std::vector<std::string>& tokens,
                                Database& db) {
    if (tokens.size() < 3)
        return "-Error: RENAME requires old key and new key\r\n";
    if (db.rename(tokens[1], tokens[2]))
        return "+OK\r\n";
    return "-Error: Key not found or rename failed\r\n";
}

//--------------------LIST COMMANDS--------------------

// list length command
static std::string handleListLength(const std::vector<std::string>& tokens,
                                    Database& db) {
    if (tokens.size() < 2) {
        return "-Error: LLEN requires Key\r\n";
    }
    ssize_t len = db.llen(tokens[1]);
    return ":" + std::to_string(len) + "\r\n";
}

// get list values command
static std::string handleListRange(const std::vector<std::string>& tokens,
                                   Database& db) {
    if (tokens.size() < 4) {
        return "-Error: LRANGE requires key, start and stop\r\n";
    }

    try {
        int start = std::stoi(tokens[2]);
        int stop = std::stoi(tokens[3]);

        auto elems = db.lrange(tokens[1], start, stop);

        std::string response;
        response += "*" + std::to_string(elems.size()) + "\r\n";

        for (const auto& e : elems) {
            response += "$" + std::to_string(e.size()) + "\r\n";
            response += e + "\r\n";
        }

        return response;

    } catch (const std::exception&) {
        return "-Error: Invalid range\r\n";
    }
}

// push value to the left of the list
static std::string handleLeftPush(const std::vector<std::string>& tokens,
                                  Database& db) {
    if (tokens.size() < 3) {
        return "-Error: LPUSH requires key and value\r\n";
    }

    for (size_t i = 2; i < tokens.size(); i++) {
        db.lpush(tokens[1], tokens[i]);
    }

    ssize_t len = db.llen(tokens[1]);

    return ":" + std::to_string(len) + "\r\n";
}

// push value to the right of the list
static std::string handleRightPush(const std::vector<std::string>& tokens,
                                   Database& db) {
    if (tokens.size() < 3) {
        return "-Error: RPUSH requires key and value\r\n";
    }

    for (size_t i = 2; i < tokens.size(); ++i) {
        db.rpush(tokens[1], tokens[i]);
    }

    ssize_t len = db.llen(tokens[1]);

    return ":" + std::to_string(len) + "\r\n";
}

// remove value from the left of the list
static std::string handleLeftPop(const std::vector<std::string>& tokens,
                                 Database& db) {
    if (tokens.size() < 2) {
        return "-Error: LPOP requires key\r\n";
    }
    std::string val;
    if (db.lpop(tokens[1], val))
        return "$" + std::to_string(val.size()) + "\r\n" + val + "\r\n";
    return "$-1\r\n";
}

// remove value from the right of the list
static std::string handleRightPop(const std::vector<std::string>& tokens,
                                  Database& db) {
    if (tokens.size() < 2) {
        return "-Error: RPOP requires key\r\n";
    }
    std::string val;
    if (db.rpop(tokens[1], val))
        return "$" + std::to_string(val.size()) + "\r\n" + val + "\r\n";
    return "$-1\r\n";
}

// remove matching values from the list
static std::string handleListRemove(const std::vector<std::string>& tokens,
                                    Database& db) {
    if (tokens.size() < 4) {
        return "-Error: LREM requires key, count and value\r\n";
    }

    try {
        int count = std::stoi(tokens[2]);
        std::string value = tokens[3];

        int removed = db.lrem(tokens[1], count, value);

        return ":" + std::to_string(removed) + "\r\n";

    } catch (const std::exception&) {
        return "-Error: Invalid count\r\n";
    }
}

// get value at a list index
static std::string handleListIndex(const std::vector<std::string>& tokens,
                                   Database& db) {
    if (tokens.size() < 3) {
        return "-Error: LINDEX requires key and index\r\n";
    }

    try {
        int index = std::stoi(tokens[2]);
        std::string value;

        if (db.lindex(tokens[1], index, value)) {
            return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
        }

        return "$-1\r\n";

    } catch (const std::exception&) {
        return "-Error: Invalid index\r\n";
    }
}

// replace value at a list index
static std::string handleListSet(const std::vector<std::string>& tokens,
                                 Database& db) {
    if (tokens.size() < 4) {
        return "-Error: LSET requires key, index and value\r\n";
    }

    try {
        int index = std::stoi(tokens[2]);

        if (db.lset(tokens[1], index, tokens[3])) {
            return "+OK\r\n";
        }

        return "-Error: Index out of range\r\n";

    } catch (const std::exception&) {
        return "-Error: Invalid index\r\n";
    }
}

//--------------------HASH COMMANDS--------------------

static std::string handleHset(const std::vector<std::string>& tokens,
                              Database& db) {
    if (tokens.size() < 4) {
        return "-Error: HSET requires key, field and value\r\n";
    }

    db.hset(tokens[1], {{tokens[2], tokens[3]}});
    return ":1\r\n";
}

static std::string handleHget(const std::vector<std::string>& tokens,
                              Database& db) {
    if (tokens.size() < 3) {
        return "-Error: HGET requires key and field\r\n";
    }
    std::string value;
    if (db.hget(tokens[1], tokens[2], value)) {
        return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
    }
    return "$-1\r\n";
}

static std::string handleHexists(const std::vector<std::string>& tokens,
                                 Database& db) {
    if (tokens.size() < 3) {
        return "-Error: HEXISTS requires key and field\r\n";
    }
    bool exists = db.hexists(tokens[1], tokens[2]);
    return ":" + std::to_string(exists ? 1 : 0) + "\r\n";
}

static std::string handleHdel(const std::vector<std::string>& tokens,
                              Database& db) {
    if (tokens.size() < 3) {
        return "-Error: HDEL requires key and field\r\n";
    }
    bool res = db.hdel(tokens[1], tokens[2]);
    return ":" + std::to_string(res ? 1 : 0) + "\r\n";
}

static std::string handleHgetall(const std::vector<std::string>& tokens,
                                 Database& db) {}

static std::string handleHkeys(const std::vector<std::string>& tokens,
                               Database& db) {}

static std::string handleHvals(const std::vector<std::string>& tokens,
                               Database& db) {}

static std::string handleHlen(const std::vector<std::string>& tokens,
                              Database& db) {}

static std::string handleHmset(const std::vector<std::string>& tokens,
                               Database& db) {}

std::string CommandHandler::processCommand(const std::string& commandLine) {
    auto tokens = parseRespCommand(commandLine);

    if (tokens.empty()) {
        return "-Error: Empty command\r\n";
    }

    std::string cmd = tokens[0];

    for (char& c : cmd) {
        c = std::toupper(c);
    }

    // Get the shared database instance
    Database& db = Database::getInstance();

    if (cmd == "PING") {
        return handlePing(tokens, db);
    } else if (cmd == "ECHO") {
        return handleEcho(tokens, db);
    } else if (cmd == "FLUSHALL") {
        return handleFlushAll(tokens, db);
    } else if (cmd == "SET") {
        return handleSet(tokens, db);
    } else if (cmd == "GET") {
        return handleGet(tokens, db);
    } else if (cmd == "KEYS") {
        return handleKeys(tokens, db);
    } else if (cmd == "TYPE") {
        return handleType(tokens, db);
    } else if (cmd == "DEL" || cmd == "UNLINK") {
        return handleDeleteOrUnlink(tokens, db);
    } else if (cmd == "EXPIRE") {
        return handleExpire(tokens, db);
    } else if (cmd == "RENAME") {
        return handleRename(tokens, db);
    } else if (cmd == "LLEN") {
        return handleListLength(tokens, db);
    } else if (cmd == "LRANGE") {
        return handleListRange(tokens, db);
    } else if (cmd == "LPUSH") {
        return handleLeftPush(tokens, db);
    } else if (cmd == "RPUSH") {
        return handleRightPush(tokens, db);
    } else if (cmd == "LPOP") {
        return handleLeftPop(tokens, db);
    } else if (cmd == "RPOP") {
        return handleRightPop(tokens, db);
    } else if (cmd == "LREM") {
        return handleListRemove(tokens, db);
    } else if (cmd == "LINDEX") {
        return handleListIndex(tokens, db);
    } else if (cmd == "LSET") {
        return handleListSet(tokens, db);
    } else if (cmd == "HSET") {
        return handleHset(tokens, db);
    } else if (cmd == "HGET") {
        return handleHget(tokens, db);
    } else if (cmd == "HEXISTS")
        return handleHexists(tokens, db);
    else if (cmd == "HDEL") {
        return handleHdel(tokens, db);
    } else if (cmd == "HGETALL") {
        return handleHgetall(tokens, db);
    } else if (cmd == "HKEYS") {
        return handleHkeys(tokens, db);
    } else if (cmd == "HVALS") {
        return handleHvals(tokens, db);
    } else if (cmd == "HLEN") {
        return handleHlen(tokens, db);
    } else if (cmd == "HMSET") {
        return handleHmset(tokens, db);
    } else {
        return "-Error: Unknown command\r\n";
    }
}