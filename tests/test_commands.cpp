#include "../include/Database.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main() {
    Database& db = Database::getInstance();

    db.flushAll();

    // ---------------- String operations ----------------

    // SET / GET
    db.set("name", "Divine");
    db.set("full name", "Divine Eboigbe");

    std::string value;

    assert(db.get("name", value));
    assert(value == "Divine");

    assert(db.get("full name", value));
    assert(value == "Divine Eboigbe");

    // Missing key
    std::string missing;
    assert(!db.get("does_not_exist", missing));

    // TYPE
    assert(db.type("name") == "string");
    assert(db.type("does_not_exist") == "none");

    // KEYS
    auto allKeys = db.keys();
    assert(!allKeys.empty());

    // RENAME
    assert(db.rename("name", "username"));

    std::string renamedValue;
    assert(db.get("username", renamedValue));
    assert(renamedValue == "Divine");

    std::string oldValue;
    assert(!db.get("name", oldValue));

    // DEL
    assert(db.del("username"));
    assert(!db.del("username"));

    // EXPIRE
    db.set("temporary", "hello");
    assert(db.expire("temporary", 10));

    // Cannot expire missing key
    assert(!db.expire("missing", 10));

    // ---------------- List operations ----------------

    db.flushAll();

    // LPUSH
    db.lpush("names", "Alice");
    db.lpush("names", "Bob");
    db.lpush("names", "Charlie");

    assert(db.llen("names") == 3);

    // Expected:
    // ["Charlie", "Bob", "Alice"]

    // LINDEX
    std::string listValue;

    assert(db.lindex("names", 0, listValue));
    assert(listValue == "Charlie");

    assert(db.lindex("names", 1, listValue));
    assert(listValue == "Bob");

    // Negative index
    assert(db.lindex("names", -1, listValue));
    assert(listValue == "Alice");

    // Invalid index
    assert(!db.lindex("names", 10, listValue));

    // RPUSH
    db.rpush("names", "David");

    assert(db.llen("names") == 4);

    assert(db.lindex("names", -1, listValue));
    assert(listValue == "David");

    // LRANGE
    auto range = db.lrange("names", 1, 2);

    assert(range.size() == 2);
    assert(range[0] == "Bob");
    assert(range[1] == "Alice");

    // LSET
    assert(db.lset("names", 1, "Daniel"));

    assert(db.lindex("names", 1, listValue));
    assert(listValue == "Daniel");

    // LPOP
    std::string popped;

    assert(db.lpop("names", popped));
    assert(popped == "Charlie");

    // RPOP
    assert(db.rpop("names", popped));
    assert(popped == "David");

    // Remaining:
    // ["Daniel", "Alice"]

    assert(db.llen("names") == 2);

    // LREM
    db.rpush("numbers", "1");
    db.rpush("numbers", "2");
    db.rpush("numbers", "1");
    db.rpush("numbers", "3");
    db.rpush("numbers", "1");

    // ["1", "2", "1", "3", "1"]

    // Remove first two "1"s
    assert(db.lrem("numbers", 2, "1") == 2);

    auto numbers = db.lrange("numbers", 0, -1);

    assert(numbers.size() == 3);
    assert(numbers[0] == "2");
    assert(numbers[1] == "3");
    assert(numbers[2] == "1");

    // Remove remaining "1"
    assert(db.lrem("numbers", 0, "1") == 1);

    numbers = db.lrange("numbers", 0, -1);

    assert(numbers.size() == 2);
    assert(numbers[0] == "2");
    assert(numbers[1] == "3");

    // Missing list
    assert(db.llen("missing_list") == 0);

    std::string missingPop;
    assert(!db.lpop("missing_list", missingPop));
    assert(!db.rpop("missing_list", missingPop));

    // ---------------- FLUSHALL ----------------

    db.set("one", "1");
    db.set("two", "2");

    assert(db.flushAll());

    std::string flushedValue;

    assert(!db.get("one", flushedValue));
    assert(!db.get("two", flushedValue));

    assert(db.llen("names") == 0);
    assert(db.llen("numbers") == 0);

    std::cout << "All database tests passed!\n";

    return 0;
}