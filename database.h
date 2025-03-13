#include <sqlite3.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <QMessageBox>
#include <ctime>
#include <tuple>
#include <vector>

class database {
private:
    sqlite3 *db = nullptr;
    std::string path;
    bool initDatabase(const std::string &databasePath);
    mutable std::vector<std::tuple<std::string, std::string, std::string, int>> values;

public:
    database(const std::string &databasePath);
    ~database();
    bool executeQuery(const std::string &query);
    bool insert(const std::string &launcher, const std::string &exe, const std::string &icon, const int &lastPlayed);
    int timestamp();
    std::vector<std::tuple<std::string, std::string, std::string, int>> fetch();
    const std::vector<std::tuple<std::string, std::string, std::string, int>>& getValues() const;
};

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

bool database::insert(const std::string &launcher, const std::string &exe, const std::string &icon, const int &lastPlayed) {
    std::string insertQuery = "INSERT INTO games (launcher, exe, icon, lastPlayed) VALUES ('" +
                              launcher + "', '" + exe + "', '" + icon + "', " + std::to_string(lastPlayed) + ");";
    return executeQuery(insertQuery);
}

int database::timestamp() {
    return static_cast<int>(std::time(nullptr));
}

std::vector<std::tuple<std::string, std::string, std::string, int>> database::fetch() {
    std::vector<std::tuple<std::string, std::string, std::string, int>> tableValues;
    std::string query = "SELECT launcher, exe, icon, lastPlayed FROM games;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return tableValues;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string launcher = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string exe = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string icon = (sqlite3_column_text(stmt, 2) != nullptr) 
                           ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) 
                           : "";
        int lastPlayed = sqlite3_column_int(stmt, 3);

        tableValues.emplace_back(launcher, exe, icon, lastPlayed);
    }

    sqlite3_finalize(stmt);
    values = tableValues;
    return tableValues;
}

const std::vector<std::tuple<std::string, std::string, std::string, int>>& database::getValues() const {
    return values;
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

    // Insert sample data
    int currentTime = db.timestamp();
    db.insert("Steam", "/path/to/game1.exe", "/path/to/icon1.png", currentTime);
    db.insert("Epic", "/path/to/game2.exe", "/path/to/icon2.png", currentTime - 3600);

    // Fetch and display data
    auto games = db.fetch();
    std::cout << "Games in database:\n";
    for (const auto &game : games) {
        std::cout << "Launcher: " << std::get<0>(game)
                  << ", EXE: " << std::get<1>(game)
                  << ", Icon: " << std::get<2>(game)
                  << ", Last Played: " << std::get<3>(game)
                  << std::endl;
    }

    return 0;
}
