#include "gamebargui.h"
#include "database.h"
#include <iostream>
#include <QPushButton>
#include <QIcon>

gamebargui::gamebargui() : QMainWindow()
{
    gameDataBase = new database("database/game.db");
    QScreen *screen = QApplication::primaryScreen();
    QSize screenSize = screen->size();

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    barLayout = new QVBoxLayout(centralWidget);


    setGeometry(screenSize.width(), 0, screenSize.width() / 3, screenSize.height() / 3);
    setStyleSheet("background-color:rgba(0,0,0,0.3); border-radius:10;");
    setWindowFlags(Qt::Tool);
    show();

    std::cout << "window should be showing" << std::endl;
}

gamebargui::~gamebargui()
{
}


Button::Button(QWidget *parent, QIcon icon) : QPushButton()