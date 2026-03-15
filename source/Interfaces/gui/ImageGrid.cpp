#include "ImageGrid.hpp"
#include <expected>
#include <stdexcept>

namespace cse498
{

    ImageGrid::ImageGrid(WorldBase &world, QGraphicsScene &scene, int tileSize)
        : mWorld(world), mScene(scene), mTileSize(tileSize)
    {
    }

    // helper for MapImages
    // load images into an image list
    std::expected<std::vector<QPixmap>, std::string> ImageGrid::LoadImages(const std::vector<QString> &imagePaths)
    {
        std::vector<QPixmap> imageList;
        for (const QString &imagePath : imagePaths)
        {
            QPixmap pixmap(imagePath);
            if (pixmap.isNull())
            {
                qWarning() << "Failed to load image:" << imagePath;
                return std::unexpected("Failed to load image.");
            }
            imageList.push_back(pixmap);
        }
        return imageList;
    }

    // set images to cell type in the same order the cell types were added
    void ImageGrid::MapImages(const std::vector<QString> &imagePaths)
    {

        auto expectedImageList = LoadImages(imagePaths);

        WorldGrid &grid = mWorld.GetGrid();
        const std::vector<CellType> &cellTypes = grid.GetCellTypes();

        if (expectedImageList.has_value())
        {
            std::vector<QPixmap> &imageList = expectedImageList.value();

            // check if amount of images is the same as amount of cell types (-1 because the first cell type is reserved for errors)
            if (imageList.size() != cellTypes.size() - 1)
            {
                qWarning() << "Number of images and number of cell types must be the same!";
                qWarning() << "Number of images: " << imageList.size() << " Number of cell types: " << cellTypes.size();
                for (const auto &cellType : cellTypes)
                {
                    qWarning() << QString::fromStdString(cellType.name);
                }
                return;
            }

            // map each cell type to corresponding images
            // i: cell type index; i starts at 1 to account for 'Unknown' cell type
            for (size_t i = 1; i < cellTypes.size(); i++)
            {
                // map each cell type to images in mImageList (std::vector<QPixmap> mImageList;)
                mTilePixmaps[i] = imageList[i - 1];
            }
            qDebug() << "Images successfully mapped to cell types! Yippee!";
        }
    }

    void ImageGrid::RenderGrid()
    {
        // get the grid from the WorldBase class
        WorldGrid &grid = mWorld.GetGrid();

        // loop over rows
        for (size_t y = 0; y < grid.GetHeight(); ++y)
        {
            // loop over columns
            for (size_t x = 0; x < grid.GetWidth(); ++x)
            {

                size_t cell_id = grid[x, y];

                // find the corresponding image to the cell_id
                auto it = mTilePixmaps.find(cell_id);
                if (it != mTilePixmaps.end())
                {
                    // add pixmap to scene
                    QGraphicsPixmapItem *item = mScene.addPixmap(it->second);
                    item->setPos(x * mTileSize, y * mTileSize);
                }
            }
        }
    }

    void ImageGrid::SetSceneAndView(QGraphicsView &view)
    {
        // set up the scene rect based on the grid dimensions
        const WorldGrid &grid = mWorld.GetGrid();
        mScene.setSceneRect(0, 0, grid.GetWidth() * mTileSize, grid.GetHeight() * mTileSize);

        // set up the view size and scene
        view.setFixedSize(mScene.sceneRect().width(), mScene.sceneRect().height());
        view.setScene(&mScene);

        view.show();
    }
}