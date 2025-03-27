#include "database.h"
#include "platform.h"
#include "gui/gamebargui.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
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

    bool first = fs::exists("database/config.txt");

    if (first)
    {
        std::ofstream configFile("database/config.txt");
        configFile << "THIS FILE ALLOWS THE PROGRAM TO KNOW THAT IT HAS BEEN RAN AND DOESN'T NEED TO INITIALLIZE AGAIN (DON'T DELETE)";
        db.extractAllGames();
    }

    json j = db.getData();

    if (j["games"].empty())
    {
        std::cerr << "No games found from launchers" << std::endl;
    }

    Gamebar bar;
    bar.loadGames();
    // Temp run to test
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
