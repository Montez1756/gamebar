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
QPushButton *createPageButton(const std::string &type, pageFunction page, QWidget *parent = nullptr);
private slots:
    void loadHomePage();
    void loadSettingsPage();

};

#endif
