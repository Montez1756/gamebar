#include "database.h"
// #include "app.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "platform.h"
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

void run(json &j, std::string gameName)
{
    for (const auto game : j["games"])
    {
        if (game["name"] == gameName)
        {
            std::string launchCommand = j["launchers"][platform][game["launcher"]]["launch_cmd"];
            size_t pos = launchCommand.find("{id}");
            if (pos != std::string::npos)
            {
                launchCommand.replace(pos, 4, game["id"]);
                system(launchCommand.c_str());
                return;
            }
        }
        else
        {
        }
    }
    std::cerr << "Could not find game " << gameName << std::endl;
}

int main()
{
    // App app;
    Database db("database/games.json");
    
    //CHANGE TO CHECK IF CONFIG EXISTS FOR FIRST LAUNCH CHECK INSTEAD
    std::ifstream configFile("config.txt");
    if (!configFile.is_open())
    {
        std::cerr << "Warning: config.txt not found." << std::endl;
        return -1;
    }
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(configFile, line))
    {
        lines.push_back(line);
    }

    configFile.close();

    std::string firstLaunch = lines[0];
    size_t pos = firstLaunch.find(":");
    bool first = (firstLaunch.substr(pos + 1) != "true");
    if (first)
    {
        lines.push_back("os:" + platform);
        json &j = db.getData();
        for (auto &[launcherName, launcherConfig] : j["launchers"][platform].items())
        {
            std::string path = launcherConfig["path"];
            if (platform == "linux" && !path.empty() && path.find("~") != std::string::npos)
            {
                const char *homeDir = getenv("HOME");
                if (!homeDir)
                    throw std::runtime_error("HOME environment variable not set");

                path = path.replace(0,1,homeDir);
                j["launchers"][platform][launcherName]["path"] = path;

                db.save();
            }
            if (fs::exists(path))
            {
                std::string launcherLine = launcherName + ":" + path;
                lines.push_back(launcherLine);
            }
        }
        // lines[0] = "first:true";
        std::ofstream configFile("config.txt");
        if (!configFile.is_open())
        {
            std::cerr << "Warning: Failed to open config.txt" << std::endl;
            return -1;
        }
        for (const auto &line : lines)
        {
            configFile << line << "\n";
        }
        configFile.close();

        db.extractAllGames();
    }

    json j = db.getData();

    if (j["games"].empty())
    {
        std::cerr << "No games found from launchers" << std::endl;
    }
    //Temp run to test
    // while (true)
    // {
    //     std::string input;

    //     std::cout << "Enter game name: " << std::endl;
    //     std::getline(std::cin, input);
    //     if (input == "q")
    //     {
    //         return 0;
    //     }
    //     else
    //     {
    //         run(j, input);
    //     }
    //     // inZOI: Creative Studio
    // }

    return 0;
}
