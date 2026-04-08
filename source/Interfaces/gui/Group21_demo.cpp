#include <QApplication>
#include "MainWindow.hpp"
#include "../source/Worlds/MazeWorld.hpp"
#include "StartScreen.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Group 21 Demo");
    app.setApplicationVersion("1.0");

    cse498::MazeWorld world;

    const std::vector<QString> imagePaths = {"images/test1.png", "images/test2.png"};
    const int tileSize = 64;

    cse498::StartScreen startScreen(world, imagePaths, tileSize);
    startScreen.show();

    return app.exec();
}