#include "QtWidgets/QApplication"
#include "QtWidgets/QGraphicsView"
#include "QtWidgets/QGraphicsScene"
#include "QtWidgets/QGraphicsPixmapItem"
#include "QtGui/QPixmap"

#include "../../Worlds/MazeWorld.hpp"
#include "ImageGrid.hpp"

#include <vector>
#include <string>
#include <unordered_map>

using namespace cse498;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // create world
    MazeWorld world;

    // create view and scene
    QGraphicsView view;
    QGraphicsScene scene;

    // tile size the same as the image dimensions
    const int tileSize = 64;

    std::vector<QString> imagePaths = {"images/test1.png", "images/test2.png"};

    // create ImageGrid
    ImageGrid imageGrid(world, scene, tileSize, imagePaths);

    // render
    imageGrid.RenderGrid();
    imageGrid.SetSceneAndView(view);

    return app.exec();
}
