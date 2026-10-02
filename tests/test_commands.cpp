#include "../include/Database.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    Database& db = Database::getInstance();

    db.flushAll();

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
    assert(!db.del("username")); // already deleted

    // EXPIRE
    db.set("temporary", "hello");
    assert(db.expire("temporary", 10));

    // Cannot expire missing key
    assert(!db.expire("missing", 10));

    // FLUSHALL
    db.set("one", "1");
    db.set("two", "2");

    assert(db.flushAll());

    std::string flushedValue;
    assert(!db.get("one", flushedValue));
    assert(!db.get("two", flushedValue));

    std::cout << "All database tests passed!\n";

    return 0;
}