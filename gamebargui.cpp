#include "gamebargui.h"
#include <vector>
#include <string>
#include <QMainWindow>
#include "platform.h"
#include <QWidget>
#include <QLabel>
#include "nlohmann/json.hpp"
#include <windows.h>
#include <QIcon>
#include <QVBoxLayout>
#include <QPixmap>
#include <QImage>
#include <QFile>
#include <QPainter>
#include <QPaintEngine>

Gamebar::Gamebar(Database &globalDatabase) : db(&globalDatabase)
{
    loadGames();
    window.setWindowFlags(Qt::Tool);
    if(!window.layout()->isEmpty())
    {
        window.layout()->addWidget(games[0]);
    }
    window.show();
}

void Gamebar::loadGames()
{
    json &j = db->getData();
    for (auto game : j["games"])
    {
        if (game.contains("launcher"))
        {
            games.push_back(new Game(j["launchers"][platform][game["launcher"]], game));
        }
        else{
            games.push_back(new Game(game));
        }
    }
}

Game::Game(json &launcherObject, json &gameObject) : launcherOb(&launcherObject), gameOb(&gameObject)
{
    initGame();
}
Game::Game(json &gameObject) : gameOb(&gameObject)
{
    initGame();
}
/*
/////////////////////////////
CHANGE DATABASE GAME EXTRACTORS TO MATCH THIS
////////////////////////////
*/
void Game::initGame(){
    displayName = gameOb->at("name");
    exePath = "C:\\Users\\efish\\Downloads\\tinytask.exe";
    if(launcherOb != nullptr)
    {
        launchCMD = launcherOb->at("launch_cmd");
    }
    else{
        launchCMD = exePath;
    }
    if(gameOb->contains("icon"))
    {
        iconPath = gameOb->at("icon");
    }
    else
    {
        iconPath = "C:\\Users\\efish\\Downloads\\tinytask.exe";
    }
    layout = new QVBoxLayout(this);
    nameLabel = new QLabel(displayName.c_str(), this);
    iconLabel = new QLabel(this);

    if (iconPath != "" && iconPath.substr(iconPath.size() - 4) == ".exe")
    {
        // icon = new QIcon(iconPath.c_str());
        iconLabel->setPixmap(QPixmap(exePath.c_str()));
    }
    else if(iconPath.substr(iconPath.size() - 4) == ".exe")
    {
        iconLabel->setPixmap(extractWindowsIconAsPixmap(exePath));
    }
    else
    {
        iconLabel->setPixmap(QPixmap("gui/images/defaults/default_icon.png"));
    }

    layout->addWidget(iconLabel);
    layout->addWidget(nameLabel);
    setLayout(layout);
}


QPixmap Game::hiconToQPixmap(HICON hIcon)
{
    if (!hIcon)
    return QPixmap("gui/images/defaults/default_icon.png");
    
    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo))
    return QPixmap("gui/images/defaults/default_icon.png");
    
    // Get the bitmap info
    BITMAP bmp;
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp);
    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    
    // Create QImage
    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    
    // Get HDC
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, iconInfo.hbmColor);
    
    // Copy bitmap data into QImage
    BITMAPINFOHEADER bi;
    memset(&bi, 0, sizeof(bi));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; // Negative to keep the correct orientation
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    
    GetDIBits(hdc, iconInfo.hbmColor, 0, height, image.bits(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    
    // Clean up
    SelectObject(memDC, oldBmp);
    DeleteDC(memDC);
    ReleaseDC(NULL, hdc);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    DestroyIcon(hIcon);
    
    return QPixmap::fromImage(image);
}

QPixmap Game::extractWindowsIconAsPixmap(const std::string &exePath)
{
    HICON hIcon;
    if (ExtractIconExA(exePath.c_str(), 0, &hIcon, nullptr, 1) > 0 && hIcon)
    {
        return hiconToQPixmap(hIcon);
    }
    return QPixmap("gui/images/defaults/default_icon.png");
}
