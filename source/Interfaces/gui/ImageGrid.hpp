/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A class that replaces certain cell types with corresponding images (ie. replaces '#' with a road tile)
 **/

#pragma once

#include <vector>
#include <string>
#include <expected>

#include "../../core/WorldGrid.hpp"
#include "../../core/WorldBase.hpp"

// needed when using cmake
// #include <QApplication>
// #include <QGraphicsView>
// #include <QGraphicsScene>
// #include <QGraphicsPixmapItem>
// #include <QPixmap>

// needed when using make ;-;
#include "../../../third-party/qt/include/QtWidgets/QApplication"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsView"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsScene"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsPixmapItem"
#include "../../../third-party/qt/include/QtGui/QPixmap"

namespace cse498
{
    // find example usage at Group-21/MadiQtTest.cpp
    class ImageGrid
    {
    private:
        WorldBase &mWorld;
        QGraphicsScene &mScene = *(new QGraphicsScene());
        int mTileSize{32}; // default tile size
        std::unordered_map<size_t, QPixmap> mTilePixmaps{};
        // std::vector<QPixmap> mImageList;

    public:
        ImageGrid() = delete;
        virtual ~ImageGrid() = default;

        ImageGrid(WorldBase &world, QGraphicsScene &scene, int tileSize);
        std::expected<std::vector<QPixmap>, std::string> LoadImages(const std::vector<QString> &mImagePaths);
        // bool LoadImages(const std::vector<QString> &mImagePaths);
        void MapImages(const std::vector<QString> &imagePaths);
        void RenderGrid();
        void SetSceneAndView(QGraphicsView &view);
    };
}