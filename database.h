#ifndef DATABASE_H
#define DATABASE_H

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

using json = nlohmann::json;

class Database
{

private:
    json data;
public:
    Database(const std::string &databasePath);

};

#endif