#include <string>
#include <unordered_map>

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

    std::unordered_map<std::string, std::string> store;
};