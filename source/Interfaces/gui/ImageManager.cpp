#include "ImageManager.hpp"
#include <stdexcept>
#include <algorithm>
#include <expected>
#include <optional>


namespace cse498 {


/// ImageManager owns QGraphicsScene, mScene is a reference by mOwnedScene
ImageManager::ImageManager()
    : mOwnedScene(), mScene(mOwnedScene) {}

/// ImageManager does not own scene, use this to share scene with other components
ImageManager::ImageManager(QGraphicsScene &scene)
    : mOwnedScene(), mScene(scene) {}

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

std::optional<QPixmap> ImageManager::GetImage(const QString &imageId) const {
  auto it = mImages.find(imageId);
  if (it == mImages.end()) {

    return std::nullopt;  // was throwing, now consistent with Load/Remove
  }
  return it->second;
}

bool ImageManager::HasImage(const QString &imageId) const {
  return mImages.contains(imageId);
}

bool ImageManager::Remove(const QString &imageId) {
  // std::erase_if with lambda (Advanced C++ Features)
  const size_t removed = std::erase_if(mImages,
      [&imageId](const auto &pair) { return pair.first == imageId; });

  if (removed == 0) {
    qWarning() << "Error: Image '" << imageId << "' not found";
    return false;  // was void, now consistent with Load/GetImage
  }

  qDebug() << "Removed image '" << imageId << "'";
  return true;
}

void ImageManager::Clear() {
  mImages.clear();
}

} // namespace cse498}