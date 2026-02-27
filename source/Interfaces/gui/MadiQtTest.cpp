#include "../../../third-party/qt/include/QtWidgets/QApplication"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsView"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsScene"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsPixmapItem"
#include "../../../third-party/qt/include/QtGui/QPixmap"

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
