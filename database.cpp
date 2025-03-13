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

// Constructor
database::database(const std::string &databasePath) : path(databasePath) {
    if (!initDatabase(databasePath)) {
        QString error = "Failed to initialize database " + QString::fromStdString(databasePath) + ":\n" +
                        (db ? sqlite3_errmsg(db) : "Unknown error");
        QMessageBox::critical(nullptr, "Error!", error);
    }
}

// Initialize database
bool database::initDatabase(const std::string &databasePath) {
    int rc = sqlite3_open(databasePath.c_str(), &db);
    return rc == SQLITE_OK;
}

// Destructor to close the database
database::~database() {
    if (db) {
        sqlite3_close(db);
    }
}

bool database::executeQuery(const std::string &query) {
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        QMessageBox::critical(nullptr, "Error:", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool database::insert(const std::string &table, const std::vector<std::string> &values) {
    // Step 1: Get column names dynamically
    std::vector<std::string> columns;
    std::string pragmaQuery = "PRAGMA table_info(" + table + ");";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, pragmaQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error fetching table columns: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string columnName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        columns.push_back(columnName);
    }
    sqlite3_finalize(stmt);

    // Step 2: Ensure values match columns count
    if (columns.size() != values.size()) {
        std::cerr << "Error: Mismatch between table columns and provided values!" << std::endl;
        return false;
    }

    // Step 3: Build the INSERT query dynamically
    std::string insertQuery = "INSERT INTO " + table + " (";
    std::string valuesPart = "VALUES (";

    for (size_t i = 0; i < columns.size(); ++i) {
        insertQuery += columns[i];
        valuesPart += "'" + values[i] + "'";

        if (i < columns.size() - 1) {
            insertQuery += ", ";
            valuesPart += ", ";
        }
    }

    insertQuery += ") " + valuesPart + ");";
    tables[table].emplace_back(values);
    return executeQuery(insertQuery);
}

int database::timestamp() {
    return static_cast<int>(std::time(nullptr));
}

std::vector<std::vector<std::string>> database::fetch(const std::string &table) {
    std::vector<std::vector<std::string>> tableValues;

    // Get column names dynamically
    std::vector<std::string> columns;
    std::string pragmaQuery = "PRAGMA table_info(" + table + ");";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, pragmaQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error fetching table columns: " << sqlite3_errmsg(db) << std::endl;
        return tableValues;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string columnName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        columns.push_back(columnName);
    }
    sqlite3_finalize(stmt);

    // Build dynamic SELECT query
    std::string query = "SELECT * FROM " + table + ";";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return tableValues;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<std::string> rowValues;
        for (size_t i = 0; i < columns.size(); ++i) {
            const char *text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
            rowValues.push_back(text ? text : "");
        }

        // Store as BaseVuple dynamically
        tableValues.emplace_back(rowValues);
    }

    sqlite3_finalize(stmt);
    tables[table] = tableValues;
    return tableValues;
}

std::map<std::string, std::vector<std::vector<std::string>>> &database::getValues() {
    return tables;
}

bool database::saveChanges(const std::string &table) {
    std::string clearQuery = "DELETE FROM " + table + ";";
    if (executeQuery(clearQuery)) {
        for (const auto &record : tables[table]) {
            std::vector<std::string> values;
            for (size_t i = 0; i < record.size(); ++i) {
                auto value = record[i];
                if (!value.empty()) {
                    values.push_back(value);
                }
                else {
                    values.push_back("NULL"); // Handle non-string values appropriately
                }
            }
            if (!insert(table, values)) {
                return false;
            }
        }
    }
    else {
        return false;
    }
    return true;
}

int main() {
    database db("database/games.db");

    // Create the table
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS games (
            launcher TEXT NOT NULL,
            exe TEXT NOT NULL,
            icon TEXT,
            lastPlayed INT
        );
    )";
    db.executeQuery(createTableQuery);

    createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS launchers (
            name TEXT NOT NULL,
            gamesPath TEXT NOT NULL
        );
    )";
    db.executeQuery(createTableQuery);
    db.insert("games", {"epic", "gta.exe", "idk/idk.jpg", "12131"});
    for(const auto info: db.getValues()["games"][0]){
        std::cout << info << std::endl;
    }
    db.fetch("games");
    for(const auto info: db.getValues()["games"][0][0]){
        std::cout << info << std::endl;
    }
    return 0;
}