#include "gui.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>

Gui::Gui() : QObject()
{

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect size = screen->geometry();
    invsWindow.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    invsWindow.setAttribute(Qt::WA_TranslucentBackground);
    invsWindow.setWindowTitle("Gamebar");
    invsWindow.setGeometry((size.width() - (size.height() / 3)),0,size.width() / 3, size.height() / 3);

    window = new QWidget(&invsWindow);
    window->resize(invsWindow.size());
    window->setObjectName("mainWindow");
    styleFromFile(window, window->objectName().toStdString());

    mainLayout = new QHBoxLayout(window);
    window->setLayout(mainLayout);

    pageContainer = new QWidget(window);
    gameContainer = new QWidget(window);

    mainLayout->addWidget(pageContainer, 0, Qt::AlignLeft);
    mainLayout->addWidget(gameContainer, 1);

    pageLayout = new QVBoxLayout(pageContainer);
    gameLayout = new QVBoxLayout(gameContainer);

    pageContainer->setLayout(pageLayout);
    gameContainer->setLayout(gameLayout);

    QPushButton *homeButton = createPageButton("Home", &Gui::loadHomePage, pageContainer, false);
    homeButton->setObjectName("pageButton");
    homeButton->setAttribute(Qt::WA_TranslucentBackground);
    styleFromFile(homeButton, homeButton->objectName().toStdString());
    pageLayout->addWidget(homeButton, 2);
    
    QPushButton *settingsButton = createPageButton("Settings", &Gui::loadSettingsPage, pageContainer, false);
    settingsButton->setObjectName("pageButton");
    settingsButton->setAttribute(Qt::WA_TranslucentBackground);
    styleFromFile(settingsButton, homeButton->objectName().toStdString());
    pageLayout->addWidget(settingsButton, 2);

    window->show();
    invsWindow.show();
}

QPushButton *Gui::createPageButton(const std::string &type, pageFunction page, QWidget *parent, bool text)
{
    QPushButton *pageButton;
    std::string iconPath = "gui/defaults/" +  type + ".png";
    if(text)
    {
        pageButton = new QPushButton(QIcon(iconPath.c_str()), type.c_str(), parent);
    }
    else
    {
        pageButton = new QPushButton(QIcon(iconPath.c_str()), "", parent);
    }

    QObject::connect(pageButton, &QPushButton::clicked, this, [this, page]() {
        if (page) (this->*page)();
    });

    return pageButton;
}

void Gui::loadHomePage(){
    std::cout << "ello nigger" << std::endl;
}

void Gui::loadSettingsPage(){
    std::cout << "ello settings nigger" << std::endl;
}

void Gui::styleFromFile(QWidget *element, const std::string &name)
{
    std::ifstream cssFile("gui/style.css");
    if (!cssFile.is_open())
    {
        std::cerr << "Failed to open CSS file" << std::endl;
        return;
    }

    std::string css;
    std::string line;
    
    while (std::getline(cssFile, line))
    {
        if (line.find(name) != std::string::npos) // Found the section
        {
            css += line + "\n"; // Add the first matching line
            
            while (std::getline(cssFile, line) && line.find("}") == std::string::npos)
            {
                css += line + "\n"; // Add the rest of the section
            }

            css += line + "\n"; // Add the closing brace
            break; // Stop reading after finding the first matching section
        }
    }

    element->setStyleSheet(css.c_str());
}

using json = nlohmann::json;
namespace fs = std::filesystem;

Game::Game(json &launcherObject, json &gameObject) : launcher(&launcherObject), game(&gameObject)
{
    initGame();
}

Game::Game(json &gameObject) : game(&gameObject)
{
    initGame();
}

void Game::initGame()
{
    if(game != nullptr)
    {
        name = game->at("name");
        id = game->at("id");
        path = game->at(path);
        if(!game->contains("icon"))
        {
            HICONToImage(path);
        }
    }
    if(launcher != nullptr)
    {
        launchCommand = launcher->at("lauch_cmd").get<std::string>();
        launchCommand.replace(launchCommand.find("{id}"), 4, id);
    }
}




#pragma pack(push, 1)
struct BMPHEADER {
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
};
#pragma pack(pop)

void SaveHBITMAPToBMP(HBITMAP hBitmap, const std::string &filename) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int imageSize = bmp.bmWidthBytes * bmp.bmHeight;
    int fileSize = sizeof(BMPHEADER) + imageSize;

    BMPHEADER header = {0};
    header.fileHeader.bfType = 0x4D42; // 'BM'
    header.fileHeader.bfSize = fileSize;
    header.fileHeader.bfOffBits = sizeof(BMPHEADER);
    header.infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    header.infoHeader.biWidth = bmp.bmWidth;
    header.infoHeader.biHeight = -bmp.bmHeight; // Negative height for top-down DIB
    header.infoHeader.biPlanes = 1;
    header.infoHeader.biBitCount = 32;
    header.infoHeader.biCompression = BI_RGB;
    header.infoHeader.biSizeImage = imageSize;

    std::ofstream file(filename.c_str(), std::ios::binary);
    if (!file) {
        std::cerr << L"Failed to open file: " << filename << std::endl;
        return;
    }

    file.write(reinterpret_cast<char*>(&header), sizeof(BMPHEADER));

    HDC hdc = GetDC(NULL);
    BITMAPINFO bi = {0};
    bi.bmiHeader = header.infoHeader;
    std::vector<uint8_t> buffer(imageSize);

    if (!buffer.empty()) {
        GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, buffer.data(), &bi, DIB_RGB_COLORS);
        file.write(reinterpret_cast<char*>(buffer.data()), imageSize);
    }

    ReleaseDC(NULL, hdc);
    file.close();
}

