#include "database.h"
// #include "app.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "platform.h"
#include <filesystem>

namespace fs = std::filesystem;

int main()
{
    // App app;
    Database db("database/games.json");

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
        json j = db.getData();
        for(const auto launcher: j["launchers"][platform]["launchers"])
        {
            if(fs::exists(launcher["path"])){
                std::string launcherConfig = launcher["name"].dump() + ":" + launcher["path"].dump();
                lines.push_back(launcherConfig);
            }
        }
        lines[0] = "first:true";
        std::ofstream configFile("config.txt");
        if(!configFile.is_open())
        {
            std::cerr << "Warning: Failed to open config.txt" << std::endl;
            return -1;
        }
        for(const auto &line: lines)
        {
            configFile << line << "\n";
        }
        configFile.close();


        db.extractAllGames();
    }
    
    json j = db.getData();
    std::cout << platform << std::endl;
    if(j["games"].empty())
    {
        std::cerr << "No games found from launchers" << std::endl;
    }
    return 0;
}