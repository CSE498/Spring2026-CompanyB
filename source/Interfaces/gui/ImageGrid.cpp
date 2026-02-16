#include "ImageGrid.hpp"

namespace cse498
{

    ImageGrid::ImageGrid(WorldBase &world, QGraphicsScene &scene, int tileSize, std::vector<QString> &mImagePaths)
        : mWorld(world), mScene(scene), mTileSize(tileSize)
    {
        if (!LoadImages(mImagePaths))
        {
            qWarning() << "Failed to load images!";
        }
        else
        {
            MapImages();
        }
    }

    // helper function
    bool ImageGrid::LoadImages(const std::vector<QString> &mImagePaths)
    {
        mImageList.clear();
        for (const QString &imagePath : mImagePaths)
        {
            QPixmap pixmap(imagePath);
            if (pixmap.isNull())
            {
                qWarning() << "Failed to load image:" << imagePath;
                return false;
            }
            mImageList.push_back(pixmap);
        }
        return true;
    }

    // set images to cell type in the same order the cell types were added
    void ImageGrid::MapImages()
    {
        WorldGrid &grid = mWorld.GetGrid();
        const std::vector<CellType> &cellTypes = grid.GetCellTypes();

        // check if amount of images is the same as amount of cell types (-1 because the first cell type is reserved for errors)
        if (mImageList.size() != cellTypes.size() - 1)
        {
            qWarning() << "Number of images and number of cell types must be the same!";
            qWarning() << "Number of images: " << mImageList.size() << " Number of cell types: " << cellTypes.size();
            for (const auto &cellType : cellTypes)
            {
                qWarning() << QString::fromStdString(cellType.name); // Assuming 'name' is a std::string
            }
            return;
        }

        // map each cell type to corresponding images
        // i = 1 to account for 'Unknown' cell type
        for (size_t i = 1; i < cellTypes.size(); i++)
        {
            // std::unordered_map<size_t, QPixmap> tilePixmaps;
            // map each cell type to images in mImageList (std::vector<QPixmap> mImageList;)
            size_t cellTypeID = i;
            mTilePixmaps[cellTypeID] = mImageList[i - 1];
        }
        qDebug() << "Images successfully mapped to cell types! Yippee!";
    }

    void ImageGrid::RenderGrid()
    {
        // get the grid from the WorldBase class
        WorldGrid &grid = mWorld.GetGrid();

        for (size_t y = 0; y < grid.GetHeight(); ++y)
        {
            for (size_t x = 0; x < grid.GetWidth(); ++x)
            {

                size_t cell_id = grid(x, y);

                auto it = mTilePixmaps.find(cell_id);
                if (it != mTilePixmaps.end())
                {
                    QGraphicsPixmapItem *item = mScene.addPixmap(it->second);
                    item->setPos(x * mTileSize, y * mTileSize);
                }
            }
        }
    }

    void ImageGrid::SetSceneAndView(QGraphicsView &view)
    {
        // set up the scene rect based on the grid dimensions
        WorldGrid &grid = mWorld.GetGrid();
        mScene.setSceneRect(0, 0, grid.GetWidth() * mTileSize, grid.GetHeight() * mTileSize);

        // set up the view size and scene
        view.setFixedSize(mScene.sceneRect().width() + 2, mScene.sceneRect().height() + 2);
        view.setScene(&mScene);

        view.show();
    }
}