#include <nlohmann/json.hpp>
#include <fstream>
#include "database.h"
#include <iostream>
#include <filesystem>
#include <map>
#include "platform.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

Database::Database(const std::string &databasePath) : path(databasePath)
{
    launcherMethods = {
        {"steam", &Database::extractSteamGames},
        {"epic games", &Database::extractEpicGames},
        // {"ea", &Database::extractEAGames},
        // {"ubisoft", &Database::extractUbisoftGames},
        // {"gog", &Database::extractGOGGames},
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
    for (const auto launcher : launcherMethods)
    {
        std::cout << "ello mate" << std::endl;
        auto launcherOb = data["launchers"][platform][launcher.first];
        std::cout << launcherOb.dump() << std::endl;
        (this->*launcher.second)(launcherOb["path"]);
    }
}

void Database::extractSteamGames(const fs::path &launcherPath)
{

    try
    {
        for (const auto &entry : fs::directory_iterator(launcherPath))
        {
            std::cout << entry.path() << std::endl;
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
                                         {"id", appID},
                                         {"path", (launcherPath / installDir).string()},
                                        {"launcher","steam"}});
            }
        }
        save();
    }

    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

void Database::extractEpicGames(const fs::path &manifestPath)
{
    try
    {
        for (const auto &entry : fs::directory_iterator(manifestPath))
        {
            if (entry.path().extension() != ".item")
                continue;

            std::ifstream file(entry.path());
            if (!file.is_open())
            {
                std::cerr << "Failed to open Epic manifest: " << entry.path() << std::endl;
                continue;
            }

            json manifest;
            try
            {
                file >> manifest;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                continue;
            }

            std::string appID = manifest.value("AppName", "");
            std::string installDir = manifest.value("InstallLocation", "");
            if (!appID.empty() && !installDir.empty())
            {
                data["games"].push_back({
                    {"id", appID},
                    {"path", installDir},
                    {"name", appID},
                    {"launcher", "epic games"}
                });
            }
        }
        save();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error during Epic Games extraction: " << e.what() << std::endl;
    }
}


json &Database::getData()
{
    return data;
}

void Database::save()
{
    std::ofstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open database file" << std::endl;
        return;
    }
    file << data.dump(4);
    file.close();
}