#include "ImageGrid.hpp"

#include <expected>
#include <stdexcept>

namespace cse498
{

    ImageGrid::ImageGrid(WorldBase &world, QGraphicsScene &scene, int tileSize)
        : mWorld(world), mScene(scene), mTileSize(tileSize) {}

    // set images to cell type in the same order the cell types were added
    void ImageGrid::MapImages(const std::vector<QString> &imagePaths)
    {
        const WorldGrid &grid = mWorld.GetGrid();
        const std::vector<CellType> &cellTypes = grid.GetCellTypes();

        if (imagePaths.size() != cellTypes.size() - 1)
        {
            // check if amount of images is the same as amount of cell types (-1
            // because the first cell type is reserved for errors)
            qWarning() << "Number of images and number of cell types must be "
                          "the same!";
            qWarning() << "Number of images: " << imagePaths.size()
                       << " Number of cell types: " << cellTypes.size();
            for (const auto &cellType : cellTypes)
            {
                qWarning() << QString::fromStdString(cellType.name);
            }
            return;
        }

        // map each cell type to corresponding images
        // i: cell type index; i starts at 1 to account for 'Unknown' cell type
        for (size_t i = 0; i < cellTypes.size() - 1; i++)
        {
            // map each cell type to images
            QString imageID = QString::number(i + 1);
            if (!mImageManager.Load(imageID, imagePaths[i]))
            {
                qWarning() << "Failed to load image: " << imagePaths[i];
                return;
            }
        }

        qDebug() << "Images successfully mapped to cell types! Yippee!";
    }

    void ImageGrid::RenderGrid()
    {
        // get the grid from the WorldBase class
        const WorldGrid &grid = mWorld.GetGrid();

        // loop over rows
        for (size_t y = 0; y < grid.GetHeight(); ++y)
        {
            // loop over columns
            for (size_t x = 0; x < grid.GetWidth(); ++x)
            {
                size_t cell_id = grid[x, y];
                QString imageID = QString::number(cell_id);

                // find the corresponding image to the cell_id
                if (mImageManager.HasImage(imageID))
                {
                    QGraphicsPixmapItem *item =
                        mScene.addPixmap(mImageManager.GetImage(imageID).value());
                    item->setPos(x * mTileSize, y * mTileSize);
                }
            }
        }
    }

    void ImageGrid::SetSceneAndView(QGraphicsView &view)
    {
        // set up the scene rect based on the grid dimensions
        const WorldGrid &grid = mWorld.GetGrid();
        mScene.setSceneRect(0, 0, grid.GetWidth() * mTileSize,
                            grid.GetHeight() * mTileSize);

        // set up the view size and scene
        view.setScene(&mScene);
        view.show();
    }

    void ImageGrid::ClearAgents()
    {
        for (auto *item : mAgentItems)
        {
            mScene.removeItem(item);
            delete item;
        }
        mAgentItems.clear();
    }

    void ImageGrid::LoadAgentImage(const QString &imagePath)
    {
        if (!mImageManager.Load("agent", imagePath))
            qWarning() << "Failed to load agent image: " << imagePath;
    }

    void ImageGrid::RenderAgents()
    {
        ClearAgents();

        if (!mImageManager.HasImage("agent"))
        {
            qWarning() << "No agent image loaded!";
            return;
        }

        QPixmap agentPixmap = mImageManager.GetImage("agent").value();

        for (size_t i = 0; i < mWorld.GetNumAgents(); i++)
        {
            const AgentBase &agent = mWorld.GetAgent(i);
            WorldPosition pos = agent.GetLocation().AsWorldPosition();

            QGraphicsPixmapItem *item = mScene.addPixmap(agentPixmap);
            item->setPos(pos.CellX() * mTileSize, pos.CellY() * mTileSize);
            mAgentItems.push_back(item);
        }
    }
} // namespace cse498