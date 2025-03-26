#include "gamebargui.h"
#include <vector>
#include <string>
#include <QMainWindow>
#include "platform.h"
#include <QWidget>
#include <QLabel>
#include "nlohmann/json.hpp"
// #include <windows.h>
#include <QIcon>
#include <QVBoxLayout>
#include <QPixmap>

Gamebar::Gamebar() : db(new Database("./database/games.db"))
{
    loadGames();
}

void Gamebar::loadGames()
{
    json &j = db->getData();
    for (auto game : j["games"])
    {
        games.push_back(new Game(j["launchers"][platform][game["launcher"]], game));
    }
}

Game::Game(json &launcherObject, json &gameObject)
{
    displayName = gameObject["name"];
    exePath = gameObject["exe"];
    launchCMD = launcherObject["launch_cmd"];
    iconPath = gameObject["exe"];
    layout = new QVBoxLayout();
    label = new QLabel(displayName.c_str());

    if (iconPath != "" && iconPath.substr(iconPath.size() - 4) == ".exe")
    {
        icon = new QIcon(iconPath.c_str());
    }
    else
    {
        icon = getIcon();
    }

    layout->addWidget(new QLabel(icon));
    layout->addWidget(label);
    setLayout(layout);
}

QIcon *Game::getIcon()
{
    return new extractWindowsIcon(exePath);
}
QIcon Game::extractWindowsIcon(const std::string &exe)
{
    HICON hIcon = ExtractIcon(NULL, exePath.c_str(), 0); // Get the first icon
    if (hIcon)
    {
        QPixmap pixmap = QPixmap::fromWinHICON(hIcon); // Convert HICON to QPixmap
        DestroyIcon(hIcon);                            // Don't forget to destroy the icon when done
        return QIcon(pixmap);                          // Convert QPixmap to QIcon
    }
    return QIcon(); // Return an empty QIcon if no icon was found
}

