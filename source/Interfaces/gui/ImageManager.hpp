#pragma once

/**
 * @file ImageManager.hpp
 *
 * Ownership model:
 *  - Default constructor: ImageManager owns its QGraphicsScene internally
 *    (mOwnedScene). mScene is a reference bound to mOwnedScene.
 *
 *  - QGraphicsScene& constructor: ImageManager does NOT own the scene.
 */

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QString>
#include <optional>
#include <unordered_map>

namespace cse498 {

class ImageManager {
 public:
  /// Creates an ImageManager that owns its own internal QGraphicsScene.
  ImageManager();

  /// Creates an ImageManager that uses an externally-owned QGraphicsScene.
  explicit ImageManager(QGraphicsScene &scene);

  ~ImageManager() = default;

  /// delete them so that it doesn't cause errors later on if there's two
  /// ImageManager instances
  ImageManager(const ImageManager &) = delete;
  ImageManager &operator=(const ImageManager &) = delete;
  ImageManager(ImageManager &&) = delete;
  ImageManager &operator=(ImageManager &&) = delete;

  /// Loads an image from disk and stores it under imageId.
  /// returns true on success, false on any failure (logs via qWarning).
  [[nodiscard]] bool Load(const QString &imageId, const QString &path);

  /// Returns the stored pixmap for imageId, or std::nullopt if not found.
  [[nodiscard]] std::optional<QPixmap> GetImage(const QString &imageId) const;

  /// Returns true if imageId is present in the manager.
  [[nodiscard]] bool HasImage(const QString &imageId) const;

  /// Removes imageId from the manager.
  /// @return true if the image existed and was removed, false if not found.
  bool Remove(const QString &imageId);

  /// Removes all stored images.
  void Clear();

 private:
  QGraphicsScene mOwnedScene;  /// used when default constructor
  QGraphicsScene &mScene;      /// refers to active scene
  std::unordered_map<QString, QPixmap> mImages;
};

}  // namespace cse498