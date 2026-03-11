// #include "../../Worlds/MazeWorld.hpp"
// #include "ImageGrid.hpp"

// #include <vector>
// #include <unordered_map>
// #include <string>
// #include <unordered_map>

// using namespace cse498;

// int main(int argc, char *argv[])
// {
//     QApplication app(argc, argv);

//     // create world
//     MazeWorld world;

//     // create view and scene
//     QGraphicsView view;
//     QGraphicsScene scene;

//     // tile size the same as the image dimensions
//     const int tileSize = 64;

//     std::vector<QString> imagePaths = {"images/test1.png", "images/test2.png"};

//     // TODO: suggestion from peer review to use map
//     // std::unordered_map<std::string, QString> imageMap = {{"floor", "images/test1.png"}, {"wall", "images/test2.png"}};

//     // create ImageGrid
//     ImageGrid imageGrid(world, scene, tileSize);

//     // render
//     imageGrid.MapImages(imagePaths);
//     imageGrid.RenderGrid();
//     imageGrid.SetSceneAndView(view);

//     return app.exec();
// }
