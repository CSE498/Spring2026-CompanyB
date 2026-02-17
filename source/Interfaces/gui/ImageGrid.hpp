/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A class that replaces certain cell types with corresponding images (ie. replaces '#' with a road tile)
 **/

#pragma once

#include <vector>
#include <string>
#include "ImageManager.hpp"
#include "../../core/WorldGrid.hpp"
#include "../../core/WorldBase.hpp"
#include "../../../third-party/qt/include/QtWidgets/QApplication"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsView"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsScene"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsPixmapItem"
#include "../../../third-party/qt/include/QtGui/QPixmap"

namespace cse498
{
    // find example usagage at Group-21/MadiQtTest.cpp
    class ImageGrid : public WorldGrid
    {
    private:
        WorldBase &mWorld;
        QGraphicsScene &mScene;
        int mTileSize;
        std::unordered_map<size_t, QPixmap> mTilePixmaps;
        std::vector<QString> mImagePaths;
        std::vector<QPixmap> mImageList;

    public:
        ImageGrid() = delete;
        virtual ~ImageGrid() = default;

        ImageGrid(WorldBase &world, QGraphicsScene &scene, int tileSize, std::vector<QString> &mImagePaths);
        bool LoadImages(const std::vector<QString> &mImagePaths);
        void MapImages();
        void RenderGrid();
        void SetSceneAndView(QGraphicsView &view);
    };
}