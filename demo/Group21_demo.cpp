#include <QApplication>
#include "MainWindow.hpp"
#include "MazeWorld.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Group 21 Demo");
    app.setApplicationVersion("1.0");

    cse498::MazeWorld world;

    const std::vector<QString> imagePaths = {"images/test1.png", "images/test2.png"};
    const int tileSize = 64;

    cse498::MainWindow window(world, imagePaths, tileSize);
    window.show();

    return app.exec();
}