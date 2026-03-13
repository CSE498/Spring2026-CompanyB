#pragma once

#include <QString>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <unordered_map>
#include <expected>

namespace cse498 {

    class ImageManager {
    public:
      	ImageManager();
        ImageManager(QGraphicsScene &scene);
        ~ImageManager();

        bool Load(const QString &imageId, const QString &path);
        const QPixmap &GetImage(const QString &imageId) const;
        bool HasImage(const QString &imageId) const;
        void Remove(const QString &imageId);
        void Clear();
        void Show(const QString &imageId, int x, int y) const;
        void SetSceneAndView(QGraphicsView &view, int width, int height);

    private:
        QGraphicsScene &mScene;
        std::unordered_map<QString, QPixmap> mImages;
        QGraphicsScene mOwnedScene;
    };

} // namespace cse498