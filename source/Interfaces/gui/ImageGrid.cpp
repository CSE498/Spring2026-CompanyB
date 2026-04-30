#include "ImageGrid.hpp"

#include <expected>
#include <stdexcept>

namespace cse498 {

ImageGrid::ImageGrid(SimWorldBase<DiseaseData> &world, QGraphicsScene &scene,
                     int tileSize)
    : mWorld(world), mScene(scene), mTileSize(tileSize) {}

void ImageGrid::MapImages(const std::vector<QString> &imagePaths) {
  const WorldGrid &grid = mWorld.GetGrid();
  const std::vector<CellType> &cellTypes = grid.GetCellTypes();

  if (imagePaths.size() != cellTypes.size() - 1) {
    qWarning() << "Number of images and number of cell types must be "
                  "the same!";
    qWarning() << "Number of images: " << imagePaths.size()
               << " Number of cell types: " << cellTypes.size();
    for (const auto &cellType : cellTypes) {
      qWarning() << QString::fromStdString(cellType.name);
    }
    return;
  }

  for (size_t i = 0; i < cellTypes.size() - 1; i++) {
    QString imageID = QString::number(i + 1);
    if (!mImageManager.Load(imageID, imagePaths[i])) {
      qWarning() << "Failed to load image: " << imagePaths[i];
      return;
    }
  }

  qDebug() << "Images successfully mapped to cell types! Yippee!";
}

void ImageGrid::RenderGrid() {
  for (auto *item : mGridItems) {
    mScene.removeItem(item);
    delete item;
  }
  mGridItems.clear();

  const WorldGrid &grid = mWorld.GetGrid();

  for (size_t y = 0; y < grid.GetHeight(); ++y) {
    for (size_t x = 0; x < grid.GetWidth(); ++x) {
      size_t cell_id = grid[x, y];
      QString imageID = QString::number(cell_id);

      if (mImageManager.HasImage(imageID)) {
        QPixmap scaled = mImageManager.GetImage(imageID).value().scaled(
            mTileSize, mTileSize, Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
        QGraphicsPixmapItem *item = mScene.addPixmap(scaled);
        item->setPos(x * mTileSize, y * mTileSize);
        mGridItems.push_back(item);
      }
    }
  }
}

void ImageGrid::SetSceneAndView(QGraphicsView &view) {
  const WorldGrid &grid = mWorld.GetGrid();
  mScene.setSceneRect(0, 0, grid.GetWidth() * mTileSize,
                      grid.GetHeight() * mTileSize);

  view.setScene(&mScene);
  view.show();
}

void ImageGrid::ClearAgents() {
  for (auto *item : mAgentItems) {
    mScene.removeItem(item);
    delete item;
  }
  mAgentItems.clear();
}

void ImageGrid::LoadAgentImage(const QString &imagePath) {
  if (!mImageManager.Load("agent", imagePath))
    qWarning() << "Failed to load agent image: " << imagePath;
}

void ImageGrid::RenderAgents() {
  ClearAgents();

  if (!mImageManager.HasImage("agent")) {
    qWarning() << "No agent image loaded!";
    return;
  }

  QPixmap agentPixmap = mImageManager.GetImage("agent").value().scaled(
      mTileSize, mTileSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  for (size_t i = 0; i < mWorld.GetNumAgents(); i++) {
    WorldPosition pos = mWorld.GetAgentState(i).position;

    QGraphicsPixmapItem *item = mScene.addPixmap(agentPixmap);
    item->setPos(pos.CellX() * mTileSize, pos.CellY() * mTileSize);
    mAgentItems.push_back(item);
  }
}
}  // namespace cse498