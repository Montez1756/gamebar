#ifndef GUI_H
#define GUI_H

#include <QObject>
#include <QMainWindow>
#include <QWidget>
#include <QScreen>
#include <QGuiApplication>
#include <vector>
#include <string>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QPushButton>
#include <QIcon>
#include <QSizePolicy>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Game;

class Gui : public QObject
{
    Q_OBJECT
private:
    std::vector<Game *> games;
    QMainWindow invsWindow;
    QWidget *window;
    QHBoxLayout *mainLayout;
    QWidget *pageContainer;
    QWidget *gameContainer;
    QVBoxLayout *pageLayout;
    QVBoxLayout *gameLayout;
    using pageFunction = void (Gui::*)();

public:
    Gui();
    QPushButton *createPageButton(const std::string &type, pageFunction page, QWidget *parent = nullptr, bool text = true);
private slots:
    void loadHomePage();
    void loadSettingsPage();
    void styleFromFile(QWidget *element, const std::string &name);
};

class Game : public QWidget
{
private:
    std::string name;
    std::string id;
    std::string path;
    std::string launchCommand;
    QVBoxLayout *layout;
    json *game = nullptr;
    json *launcher = nullptr;
    QImage *icon;
public:
    Game(nlohmann::json &launcherObject, nlohmann::json &gameObject);
    Game(nlohmann::json &gameObject);
    void initGame();
    QImage HICONToImage(const std::string &path);
};
#endif
