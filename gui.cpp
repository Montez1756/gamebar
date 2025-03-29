#include "gui.h"


Gui::Gui() : QObject()
{

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect size = screen->geometry();
    invsWindow.setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    invsWindow.setAttribute(Qt::WA_TranslucentBackground);
    invsWindow.setWindowTitle("Gamebar");
    invsWindow.setGeometry(size.width() / 3,0,size.width() / 3, size.height() / 3);

    window = new QWidget(&invsWindow);
    window->setGeometry(invsWindow.geometry());
    window->setStyleSheet("border-bottom-left-radius: 10px; border-top-left-radius: 10px; background-color:black;");
    
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

    QPushButton *homeButton = createPageButton("home", &Gui::loadHomePage, pageContainer);
    pageLayout->addWidget(homeButton);

    QPushButton *settingsButton = createPageButton("settings", &Gui::loadSettingsPage, pageContainer);
    pageLayout->addWidget(settingsButton);

    invsWindow.show();
}

QPushButton *Gui::createPageButton(const std::string &type, pageFunction page, QWidget *parent)
{
    std::string iconPath = "gui/defaults/" +  type + ".png";
    QPushButton *pageButton = new QPushButton(QIcon(iconPath.c_str()), type.c_str(), parent);

    QObject::connect(pageButton, &QPushButton::clicked, this, [this, page]() {
        if (page) (this->*page)();
    });

    return pageButton;
}
