#ifndef GUI_H
#define GUI_H

#include "platform.h"
#include <QObject>
#include <QWidget>
#include <string>
#include <vector>
#include <QMainWindow>
#include "database.h"
#include <QVBoxLayout>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Gamebar : public QObject
{
    Q_OBJECT
private:
    QMainWindow window;
    std::vector<Game *> games;
    Database *db;

public:
    Gamebar();
    void loadGames();
};

class Game : public QWidget
{
private:
    std::string displayName;
    std::string launchCMD;
    std::string iconPath;
    std::string exePath;
    QVBoxLayout *layout;
    QIcon *icon;
    QLabel *label;
public:
    Game(json &launcherObject, json &gameObject);
    QIcon *getIcon();
    QIcon extractWindowsIcon(const std::string &exe);
};
#endif