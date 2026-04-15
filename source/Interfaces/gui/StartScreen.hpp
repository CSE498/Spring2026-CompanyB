#pragma once

#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "MainWindow.hpp"
#include "WorldBase.hpp"

namespace cse498 {

class StartScreen : public QWidget {
  Q_OBJECT

 public:
  /// Marking it explicit so that it prevents accidental implicit conversions
  /// parent = nullptr is because it is the first window that pops up
  explicit StartScreen(WorldBase& world, const std::vector<QString>& imagePaths,
                       int tileSize, QWidget* parent = nullptr);

  /// Called when clicked, private bc will only be called by this class
 private slots:
  void onTrafficClicked();
  void onVirusClicked();

 private:
  void launchMainWindow(const QString& mode);

  /// reference to world
  WorldBase& mWorld;
  std::vector<QString> mImagePaths{};
  int mTileSize{};

  QPushButton* mTrafficBtn{};
  QPushButton* mVirusBtn{};
  QPushButton* mGearBtn{};
};

}  // namespace cse498