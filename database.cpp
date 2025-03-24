#include <nlohmann/json.hpp>
#include <fstream>
#include "database.h"
#include <iostream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

Database::Database(const std::string &databasePath)
{
    std::ifstream file(databasePath.c_str());
    data = json::parse(file);
    file.close();
    std::cout << data.dump() << std::endl;
    std::ifstream file("config.txt");
    std::string content;
    std::getline(file, content);

    size_t pos = content.find(":");
    if(pos != std::string::npos)
    {
        bool first = content.substr(pos) == "false";
        if(first){
            for(const auto launcher: data["games"])
            {
                std::string launcherName = launcher["name"];
                fs::directory_iterator directory(launcher["name"]);
                if(launcherName == "steam")
                {
                    for(const auto entry: directory)
                    {
                        if(entry.path().extension().string() == "acf")
                        {
                            std::ifstream gameManifest(entry.path().c_str());
                            if(gameManifest.is_open())
                            {
                                std::string temp;
                                while(gameManifest >> temp)
                                {
                                    size_t appPos = temp.find("appid");
                                    size_t installPos =  temp.find("installdir");

                                    if(appPos != std::string::npos)
                                    {
                                        
                                    }
                                    else if(installPos != std::string::npos)
                                    {
                                        
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int main(){

    Database db("database/games.json");

    return 0;
}