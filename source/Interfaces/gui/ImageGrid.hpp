/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A class that replaces certain cell types with corresponding images
 * (ie. replaces '#' with a road tile)
 **/

#pragma once

#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <expected>
#include <string>
#include <vector>

#include "../../core/WorldBase.hpp"
#include "../../core/WorldGrid.hpp"
#include "ImageManager.hpp"

namespace cse498 {

class ImageGrid {
   private:
    WorldBase& mWorld;
    QGraphicsScene& mScene = *(new QGraphicsScene());
    int mTileSize{32};  // default tile size
    ImageManager mImageManager;

   public:
    ImageGrid() = delete;
    virtual ~ImageGrid() = default;

    ImageGrid(WorldBase& world, QGraphicsScene& scene, int tileSize);
    void MapImages(const std::vector<QString>& imagePaths);
    void RenderGrid();
    void SetSceneAndView(QGraphicsView& view);
};
}  // namespace cse498