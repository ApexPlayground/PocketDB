#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class Database {
  public:
    static Database& getInstance();

    // Dump / load DB from file
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
};