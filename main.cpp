#include "database.h"
// #include "app.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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

    return 0;
}