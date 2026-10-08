
#include "../include/Database.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

void testStrings(Database& db) {
    db.flushAll();

    db.set("name", "Divine");
    db.set("full name", "Divine Eboigbe");

    std::string value;

    assert(db.get("name", value));
    assert(value == "Divine");

    assert(db.get("full name", value));
    assert(value == "Divine Eboigbe");
    assert(!db.get("missing", value));

    assert(db.type("name") == "string");
    assert(db.type("missing") == "none");

    auto keys = db.keys();
    assert(keys.size() == 2);

    assert(db.rename("name", "username"));
    assert(db.get("username", value));
    assert(value == "Divine");
    assert(!db.get("name", value));

    assert(db.del("username"));
    assert(!db.del("username"));

    db.set("temporary", "hello");
    assert(db.expire("temporary", 10));
    assert(!db.expire("missing", 10));
}

void testLists(Database& db) {
    db.flushAll();

    db.lpush("names", "Alice");
    db.lpush("names", "Bob");
    db.lpush("names", "Charlie");

    // ["Charlie", "Bob", "Alice"]
    assert(db.llen("names") == 3);

    std::string value;

    assert(db.lindex("names", 0, value));
    assert(value == "Charlie");

    assert(db.lindex("names", 1, value));
    assert(value == "Bob");

    assert(db.lindex("names", -1, value));
    assert(value == "Alice");

    assert(!db.lindex("names", 10, value));

    db.rpush("names", "David");

    assert(db.llen("names") == 4);
    assert(db.lindex("names", -1, value));
    assert(value == "David");

    auto range = db.lrange("names", 1, 2);
    assert(range.size() == 2);
    assert(range[0] == "Bob");
    assert(range[1] == "Alice");

    assert(db.lset("names", 1, "Daniel"));
    assert(db.lindex("names", 1, value));
    assert(value == "Daniel");

    assert(db.lpop("names", value));
    assert(value == "Charlie");

    assert(db.rpop("names", value));
    assert(value == "David");

    assert(db.llen("names") == 2);

    db.rpush("numbers", "1");
    db.rpush("numbers", "2");
    db.rpush("numbers", "1");
    db.rpush("numbers", "3");
    db.rpush("numbers", "1");

    assert(db.lrem("numbers", 2, "1") == 2);

    auto numbers = db.lrange("numbers", 0, -1);
    assert(numbers.size() == 3);
    assert(numbers[0] == "2");
    assert(numbers[1] == "3");
    assert(numbers[2] == "1");

    assert(db.lrem("numbers", 0, "1") == 1);

    numbers = db.lrange("numbers", 0, -1);
    assert(numbers.size() == 2);
    assert(numbers[0] == "2");
    assert(numbers[1] == "3");

    assert(db.llen("missing_list") == 0);
    assert(!db.lpop("missing_list", value));
    assert(!db.rpop("missing_list", value));
}

void testHashes(Database& db) {
    db.flushAll();

    db.hset("user", {{"name", "Divine"}, {"age", "24"}});

    assert(db.hlen("user") == 2);

    std::string value;

    assert(db.hget("user", "name", value));
    assert(value == "Divine");

    assert(db.hget("user", "age", value));
    assert(value == "24");

    assert(!db.hget("user", "missing", value));
    assert(db.hexists("user", "name"));
    assert(!db.hexists("user", "missing"));

    db.hset("user", {{"age", "25"}});

    assert(db.hget("user", "age", value));
    assert(value == "25");
    assert(db.hlen("user") == 2);

    auto fields = db.hgetall("user");
    assert(fields.size() == 2);
    assert(fields.at("name") == "Divine");
    assert(fields.at("age") == "25");

    auto keys = db.hkeys("user");
    auto values = db.hvals("user");

    assert(keys.size() == 2);
    assert(values.size() == 2);

    assert(db.hdel("user", "age"));
    assert(!db.hexists("user", "age"));
    assert(db.hlen("user") == 1);

    assert(!db.hdel("user", "missing"));
    assert(db.hlen("missing_hash") == 0);
}

void testFlushAll(Database& db) {
    db.flushAll();

    db.set("one", "1");
    db.set("two", "2");
    db.lpush("names", "Alice");
    db.hset("user", {{"name", "Divine"}});

    assert(db.flushAll());

    std::string value;

    assert(!db.get("one", value));
    assert(!db.get("two", value));
    assert(db.llen("names") == 0);
    assert(db.hlen("user") == 0);
}

int main() {
    Database& db = Database::getInstance();

    testStrings(db);
    testLists(db);
    testHashes(db);
    testFlushAll(db);

    std::cout << "All database tests passed!\n";
    return 0;
}
