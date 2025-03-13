#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <map>
#include <sqlite3.h>
#include <string>
#include <memory>
#include <variant>
#include <stdexcept>
#include <tuple>
#include <iostream>


class database
{
private:
    sqlite3 *db = nullptr;
    std::string path;
    bool initDatabase(const std::string &databasePath);
    std::map<std::string, std::vector<std::vector<std::string>>> tables;

public:
    database(const std::string &databasePath);
    ~database();
    bool executeQuery(const std::string &query);
    bool insert(const std::string &table, const std::vector<std::string> &values);
    int timestamp();
    std::vector<std::vector<std::string>> fetch(const std::string &table);
    std::map<std::string, std::vector<std::vector<std::string>>> &getValues();
    bool saveChanges(const std::string &table);
};

#endif // DATABASE_H