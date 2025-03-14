#ifndef GAMEBARGUI_H
#define GAMEBARGUI_H

#include <QMainWindow>
#include <QScreen>
#include <QApplication>
#include <QVBoxLayout>
#include "database.h"
#include <QPushButton>
#include <string>
#include <QIcon>

enum ButtonTypes
{
    loadWindow = 0,
    executeCommand,
};

class database;

class gamebargui : public QMainWindow
{
private:
    database *gameDataBase = nullptr;
    QWidget *centralWidget = nullptr;
    QVBoxLayout *barLayout = nullptr;

public:
    gamebargui();
    ~gamebargui();
};

class Button : public QPushButton
{
private:
    std::string iconPath;
    ButtonTypes type = loadWindow;
    QWidget *objectParent = nullptr;

public:
    Button(QWidget *parent = nullptr, QIcon icon)
    Button(QWidget *parent = nullptr, QIcon icon, const std::string &text);
}

#endif