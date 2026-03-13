#include "ImageManager.hpp"
#include <stdexcept>
#include <algorithm>
#include <expected>

namespace cse498 {

ImageManager::ImageManager() : mScene(mOwnedScene) {}

ImageManager::ImageManager(QGraphicsScene &scene) : mScene(scene) {}

ImageManager::~ImageManager() {}

bool ImageManager::Load(const QString &imageId, const QString &path) {
    if (imageId.isEmpty()) {
        qWarning() << "Error: Image ID is empty";
        return false;
    }
    if (path.isEmpty()) {
        qWarning() << "Error: Path cannot be empty";
        return false;
    }

    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        qWarning() << "Failed to load image:" << path;
        return false;
    }

    mImages[imageId] = pixmap;
    qDebug() << "Successfully loaded image '" << imageId << "' from" << path;
    return true;
}

const QPixmap &ImageManager::GetImage(const QString &imageId) const {
    auto it = mImages.find(imageId);
    if (it == mImages.end()) {
        throw std::out_of_range((imageId + " was not found").toStdString());
    }
    return it->second;
}

bool ImageManager::HasImage(const QString &imageId) const {
    return mImages.find(imageId) != mImages.end();
}

void ImageManager::Remove(const QString &imageId) {
    auto it = mImages.find(imageId);
    if (it == mImages.end()) {
        // TODO: determine appropriate return/error type for missing image removal
        qWarning() << "Error: Image '" << imageId << "' not found";
        return;
    }
    mImages.erase(it);
    qDebug() << "Removed image '" << imageId << "'";
}

void ImageManager::Clear() {
    mImages.clear();
}

void ImageManager::Show(const QString &imageId, int x, int y) const {
    auto it = mImages.find(imageId);
    if (it == mImages.end()) {
        qWarning() << "Error: Image '" << imageId << "' not found";
        return;
    }

    QGraphicsPixmapItem *item = mScene.addPixmap(it->second);
    item->setPos(x, y);
}

void ImageManager::SetSceneAndView(QGraphicsView &view, int width, int height) {
    mScene.setSceneRect(0, 0, width, height);
    view.setFixedSize(width, height);
    
    view.setScene(&mScene);
    view.show();
}

} // namespace cse498e cse498}