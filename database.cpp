#include "database.h"
#include <sqlite3.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <QMessageBox>
#include <ctime>
#include <tuple>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <exception>
#include <algorithm>

namespace fs = std::filesystem;

table::table(const std::string &tableName, const std::vector<std::string> &columns, const std::vector<std::map<std::string, std::string>> &values) : columnNames(columns), tableValues(values), name(tableName) {}

std::vector<std::map<std::string, std::string>> &table::getValues()
{
    return tableValues;
}
std::string &table::getName()
{
    return name;
}
std::vector<std::string> &table::getColumns()
{
    return columnNames;
}
void table::print(){
    std::cout << name << std::endl;
    for(const auto &map: tableValues)
    {
        for(const auto &pair: map)
        {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
    }
}
// Constructor
database::database(const std::string &databasePath) : path(databasePath)
{
    if (!initDatabase(databasePath))
    {
        QString error = "Failed to initialize database " + QString::fromStdString(databasePath) + ":\n" +
                        (db ? sqlite3_errmsg(db) : "Unknown error");
        QMessageBox::critical(nullptr, "Error!", error);
    }
}

// Initialize database
bool database::initDatabase(const std::string &databasePath)
{
    if (fs::exists(databasePath))
    {
        existed = true;
    }
    int rc = sqlite3_open(databasePath.c_str(), &db);
    return rc == SQLITE_OK;
}

// Destructor to close the database
database::~database()
{
    if (db)
    {
        sqlite3_close(db);
    }
}

bool database::executeQuery(const std::string &query)
{
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        QMessageBox::critical(nullptr, "Error:", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool database::createTable(const std::string &tableName, const std::string &query)
{
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        QMessageBox::critical(nullptr, "Error:", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    std::vector<std::string> columns;
    std::string pragmaQuery = "PRAGMA table_info(" + tableName + ");";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, pragmaQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        QMessageBox::critical(nullptr, "Error fetching table columns: ", sqlite3_errmsg(db));
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::string columnName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        columns.push_back(columnName);
    }
    sqlite3_finalize(stmt);

    tables.push_back(std::make_unique<table>(tableName, columns, std::vector<std::map<std::string, std::string>>{}));


    return true;
}

bool database::loadTable(const std::string &tableName)
{
    std::vector<std::string> columns;
    std::string pragmaQuery = "PRAGMA table_info(" + tableName + ");";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, pragmaQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        QMessageBox::critical(nullptr, "Error fetching table columns: ", sqlite3_errmsg(db) );
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::string columnName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        columns.push_back(columnName);
    }
    sqlite3_finalize(stmt);
    std::vector<std::vector<std::string>> values = fetch(tableName);
    std::vector<std::map<std::string, std::string>> temp;

    for (const auto &vector : values)
    {
        std::map<std::string, std::string> tempMap;
        for (int i = 0; i < vector.size(); i++)
        {
            tempMap.emplace(columns[i], vector[i]);
        }
        temp.push_back(tempMap);
    }
    tables.push_back(std::make_unique<table>(tableName, columns, temp));

    return true;
}

bool database::insert(const std::string &tableName, const std::vector<std::string> &values)
{
    table *t = nullptr;
    for (const auto &i : tables)
    {
        if (i->getName() == tableName)
        {
            t = i.get();
            break;
        }
    }
    if (!t)
        return false;
    std::vector<std::map<std::string, std::string>> tableValues = t->getValues();
    for (const auto &map : tableValues)
    {
        if (std::any_of(values.begin(), values.end(), [&map](const std::string &val)
                        { return std::any_of(map.begin(), map.end(), [&val](const auto &pair)
                                             { return pair.second == val; }); }))
        {
            return false;
        }
    }
    // Step 1: Get column names dynamically
    std::vector<std::string> columns;
    std::string pragmaQuery = "PRAGMA table_info(" + tableName + ");";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, pragmaQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        QMessageBox::critical(nullptr, "Error fetching table columns: ", sqlite3_errmsg(db) );
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::string columnName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        columns.push_back(columnName);
    }
    sqlite3_finalize(stmt);

    // Step 2: Ensure values match columns count
    if (columns.size() != values.size())
    {
        std::cerr << "Error: Mismatch between table columns and provided values!" << std::endl;
        return false;
    }

    // Step 3: Build the INSERT query dynamically
    std::string insertQuery = "INSERT INTO " + tableName + " (";
    std::string valuesPart = "VALUES (";

    for (size_t i = 0; i < columns.size(); ++i)
    {
        insertQuery += columns[i];
        valuesPart += "'" + values[i] + "'";

        if (i < columns.size() - 1)
        {
            insertQuery += ", ";
            valuesPart += ", ";
        }
    }

    insertQuery += ") " + valuesPart + ");";
    for (const auto &t : tables)
    {
        if (t->getName() == tableName)
        {
            std::map<std::string, std::string> tempMap;
            for (int i = 0; i < values.size(); i++)
            {
                tempMap.emplace(columns[i], values[i]);
            }
            t->add(tempMap);
            break;
        }
    }
    return executeQuery(insertQuery);
}

bool table::add(const std::map<std::string, std::string> &values)
{
    tableValues.emplace_back(values);
    return true; // Explicit return
}


bool changeValue()
{
}
int database::timestamp()
{
    return static_cast<int>(std::time(nullptr));
}

std::vector<std::vector<std::string>> database::fetch(const std::string &tableName)
{
    std::vector<std::vector<std::string>> tableValues;

    // Get column names dynamically
    std::vector<std::string> columns;
    std::string pragmaQuery = "PRAGMA table_info(" + tableName + ");";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, pragmaQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        QMessageBox::critical(nullptr, "Error fetching table columns: ", sqlite3_errmsg(db) );
        return tableValues;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::string columnName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        columns.push_back(columnName);
    }
    sqlite3_finalize(stmt);

    // Build dynamic SELECT query
    std::string query = "SELECT * FROM " + tableName + ";";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        QMessageBox::critical(nullptr, "Failed to prepare statement: ", sqlite3_errmsg(db));
        return tableValues;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::vector<std::string> rowValues;
        for (size_t i = 0; i < columns.size(); ++i)
        {
            const char *text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
            rowValues.push_back(text ? text : "");
        }

        // Store as BaseVuple dynamically
        tableValues.emplace_back(rowValues);
    }

    sqlite3_finalize(stmt);

    return tableValues;
}

bool database::update(const std::string &tableName, const std::map<std::string, std::string> &values, const std::map<std::string, std::string> &condition)
{
    if (values.empty() || condition.empty())
    {
        return false; // Avoid invalid SQL statements
    }

    std::string updateQuery = "UPDATE " + tableName + " SET ";

    // Construct SET clause
    for (auto it = values.begin(); it != values.end(); ++it)
    {
        updateQuery += it->first + " = '" + it->second + "'";
        if (std::next(it) != values.end())
        {
            updateQuery += ", ";
        }
    }

    // Construct WHERE clause
    std::string whereQuery = " WHERE ";
    for (auto it = condition.begin(); it != condition.end(); ++it)
    {
        whereQuery += it->first + " = '" + it->second + "'";
        if (std::next(it) != condition.end())
        {
            whereQuery += " AND ";
        }
    }

    // Final query
    std::string finalQuery = updateQuery + whereQuery + ";";

    return executeQuery(finalQuery);
}

std::vector<std::unique_ptr<table>> &database::getTables(){
    return tables;
}
