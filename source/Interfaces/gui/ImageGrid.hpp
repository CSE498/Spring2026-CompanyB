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

#include "../../Worlds/SimWorldBase.hpp"
#include "../../core/WorldGrid.hpp"
#include "ImageManager.hpp"

namespace cse498
{

    class ImageGrid
    {
    private:
        SimWorldBase<DiseaseData> &mWorld;
        QGraphicsScene &mScene = *(new QGraphicsScene());
        int mTileSize{32};
        ImageManager mImageManager;
        std::vector<QGraphicsItem *> mAgentItems;
        std::vector<QGraphicsItem *> mGridItems;

    public:
        ImageGrid() = delete;
        virtual ~ImageGrid() = default;

        ImageGrid(SimWorldBase<DiseaseData> &world, QGraphicsScene &scene, int tileSize);
        void MapImages(const std::vector<QString> &imagePaths);
        void RenderGrid();
        void RenderAgents();
        void ClearAgents();
        void LoadAgentImage(const QString &imagePath);
        void SetSceneAndView(QGraphicsView &view);
    };
} // namespace cse498