#include <nlohmann/json.hpp>
#include <fstream>
#include "database.h"
#include <iostream>
#include <filesystem>
#include <map>

using json = nlohmann::json;
namespace fs = std::filesystem;

Database::Database(const std::string &databasePath)
{
    launcherMethods = {
        {"steam", &Database::extractSteamGames},
        {"epic games", &Database::extractEpicGames},
        {"ubisoft", &Database::extractUbisoftGames},
        {"gog", &Database::extractGOGGames},
        {"ea", &Database::extractEAGames},
    };

    std::ifstream file(databasePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open database file: " << databasePath << std::endl;
        return;
    }

    try
    {
        data = json::parse(file);
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        file.close();
        return;
    }
    file.close();
}

void Database::extractAllGames()
{
    for (const auto launcher : data["launchers"])
    {
        std::string name = launcher["name"];
        if (launcherMethods.find(name) != launcherMethods.end())
        {
            (this->*launcherMethods[name])(launcher["path"]);
        }
    }
}

void Database::extractSteamGames(const fs::path &launcherPath)
{

    try
    {
        for (const auto &entry : fs::directory_iterator(launcherPath))
        {
            if (entry.path().extension() != ".acf")
                continue;

            std::ifstream gameManifest(entry.path());
            if (!gameManifest.is_open())
            {
                std::cerr << "Failed to open steam manifest: " << entry.path() << std::endl;
                continue;
            }

            std::string line;
            std::string appID, name, installDir;
            while (std::getline(gameManifest, line))
            {
                if (line.find("\"appid\"") != std::string::npos)
                {
                    size_t start = line.find('\"', line.find("\"appid\"") + 7) + 1;
                    size_t end = line.find('\"', start);
                    appID = line.substr(start, end - start);
                }
                else if (line.find("\"name\"") != std::string::npos)
                {
                    size_t start = line.find('\"', line.find("\"name\"") + 6) + 1;
                    size_t end = line.find('\"', start);
                    name = line.substr(start, end - start);
                }
                else if (line.find("\"installdir\"") != std::string::npos)
                {
                    size_t start = line.find('\"', line.find("\"installdir\"") + 12) + 1;
                    size_t end = line.find('\"', start);
                    installDir = line.substr(start, end - start);
                }
            }
            gameManifest.close();

            if (!appID.empty() && !installDir.empty() && !name.empty())
            {
                data["games"].push_back({{"name", name},
                                         {"appid", appID},
                                         {"dir", (launcherPath / installDir).string()}});
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

int main()
{
    Database db("database/games.json");

    std::ifstream configFile("config.txt");
    if (!configFile.is_open())
    {
        std::cerr << "Warning: config.txt not found." << std::endl;
        return;
    }

    std::string content;
    std::getline(configFile, content);
    configFile.close();

    size_t pos = content.find(":");
    if (pos != std::string::npos)
    {
        bool firstLaunch = (content.substr(pos + 1) == "true");
        if (firstLaunch)
        {
            db.extractAllGames();
        }
    }
    return 0;
}