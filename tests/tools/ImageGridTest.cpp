#include <catch2/catch.hpp>

#include "../../source/Interfaces/gui/ImageGrid.hpp"
#include "../../source/core/WorldBase.hpp"
#include "../../source/Worlds/MazeWorld.hpp"

TEST_CASE("Test ImageGrid Constructor", "[Interfaces]")
{
    cse498::MazeWorld world; // I'm using a MazeWorld to test currently
    QGraphicsScene scene;
    int tileSize = 64;

    std::vector<QString> imagePaths = {"images/test1.png", "images/test2.png"};

    cse498::ImageGrid grid(world, scene, tileSize, imagePaths);

    // constructor works!!!
    SUCCEED();
}