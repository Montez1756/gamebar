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


class table{

    private:

        std::vector<std::string> columnNames;
        std::vector<std::map<std::string, std::string>> tableValues;
        std::string name;

    public:
        table(const std::string &tableName, const std::vector<std::string> &columns, const std::vector<std::map<std::string, std::string>> &values);
        std::vector<std::map<std::string, std::string>> &getValues();
        std::string &getName();
        std::vector<std::string> &getColumns();
        bool add(const std::map<std::string, std::string> &values);
        void print();
};

class database
{
private:
    sqlite3 *db = nullptr;
    std::string path;
    bool initDatabase(const std::string &databasePath);
    std::vector<std::unique_ptr<table>> tables;


public:
    bool existed = false;
    database(const std::string &databasePath);
    ~database();
    bool executeQuery(const std::string &query);
    bool insert(const std::string &tableName, const std::vector<std::string> &values);
    int timestamp();
    std::vector<std::vector<std::string>> fetch(const std::string &tableName);
    std::map<std::string, std::vector<std::vector<std::string>>> &getValues();
    bool saveChanges(const std::string &tableName);
    bool update(const std::string &tableName, const std::map<std::string, std::string>& values, const std::map<std::string, std::string>& condition);
    bool createTable(const std::string &tableName, const std::string &query);
    bool loadTable(const std::string &tableName);
    std::vector<std::unique_ptr<table>> &getTables();
};


#endif // DATABASE_H