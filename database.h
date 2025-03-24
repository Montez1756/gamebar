#ifndef DATABASE_H
#define DATABASE_H

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <filesystem>
#include <map>

using json = nlohmann::json;
namespace fs = std::filesystem;

class Database
{
private:
    json data;
    
    void extractEpicGames(const fs::path &launcherPath);
    void extractSteamGames(const fs::path &launcherPath);
    void extractUbisoftGames(const fs::path &launcherPath);
    void extractGOGGames(const fs::path &launcherPath);
    void extractEAGames(const fs::path &launcherPath);
    
    using gameMethod = void (Database::*)(const fs::path&);
    std::map<std::string, gameMethod> launcherMethods;
    
public:
    Database(const std::string &databasePath);
    void extractAllGames();
    json &getData();
};

#endif