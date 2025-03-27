#ifndef GUI_H
#define GUI_H

#include "platform.h"
#include "database.h"
#include <QObject>
#include <QWidget>
#include <string>
#include <vector>
#include <QMainWindow>
#include <QVBoxLayout>
#include "nlohmann/json.hpp"
#include <windows.h>

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
    QLabel *iconLabel;
    QLabel *nameLabel;
    json *launcherOb = nullptr;
    json *gameOb;
public:
    Game(json &launcherObject, json &gameObject);
    Game(json &gameObject);
    void initGame();
    QIcon *getIcon();
    QPixmap hiconToQPixmap(HICON hIcon);
    QPixmap extractWindowsIconAsPixmap(const std::string& exePath);
};
#endif