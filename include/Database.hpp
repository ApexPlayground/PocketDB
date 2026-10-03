#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class Database {
  public:
    static Database& getInstance();

    // DB functionality commands
    bool flushAll();

    // DB key value commands
    void set(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    std::vector<std::string> keys();
    std::string type(const std::string& key);
    bool del(const std::string& key);
    bool expire(const std::string& key, int seconds);
    void purgeExpired();
    bool rename(const std::string& oldKey, const std::string& newKey);

    // DB list commands
    ssize_t llen(const std::string& key);
    void lpush(const std::string& key, const std::string& value);
    void rpush(const std::string& key, const std::string& value);
    bool lpop(const std::string& key, std::string& value);
    bool rpop(const std::string& key, std::string& value);
    int lrem(const std::string& key, int count, const std::string& value);
    bool lindex(const std::string& key, int index, std::string& value);
    bool lset(const std::string& key, int index, const std::string& value);

    // Dump / load DB for persistence
    bool dump(const std::string& filename);
    bool load(const std::string& filename);

  private:
    Database() = default;
    ~Database() = default;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    std::mutex db_mutex;

    std::unordered_map<std::string, std::string> kv_store;
    std::unordered_map<std::string, std::vector<std::string>> list_store;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>
        hash_store;

    std::unordered_map<std::string, std::chrono::steady_clock::time_point>
        expiry_map;
};