/**
 * @file AndreeaQTtest.cpp
 * @author munni
 */

#include "ImageManager.hpp"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QCoreApplication>

using namespace cse498;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QGraphicsView view;
    QGraphicsScene scene;

    ImageManager imageManager(scene);

    QString appDir = QCoreApplication::applicationDirPath();
    imageManager.Load("image1", appDir + "/../images/test1.png");
    imageManager.Load("image2", appDir + "/../images/test2.png");

    imageManager.Show("image1", 0, 0);
    imageManager.Show("image2", 64, 0);

    imageManager.SetSceneAndView(view, 256, 256);

    return app.exec();
}