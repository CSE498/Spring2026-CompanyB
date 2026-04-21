#pragma once

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <memory>

#include "../../Worlds/InfectiousWorld.hpp"
#include "../../Worlds/MazeWorld.hpp"
#include "../../Worlds/TrafficWorld.hpp"
#include "../../core/WorldBase.hpp"
#include "MainWindow.hpp"

namespace cse498 {

class StartScreen : public QWidget {
  Q_OBJECT

 public:
  explicit StartScreen(const std::vector<QString>& imagePaths, int tileSize,
                       const QString& agentImagePath,
                       QWidget* parent = nullptr);

 private slots:
  void onTrafficClicked();
  void onVirusClicked();

 private:
  void launchMainWindow(int mode);

  std::vector<QString> mImagePaths{};
  int mTileSize{};
  QString mAgentImagePath{};
  std::unique_ptr<WorldBase> mWorld{};

  QPushButton* mTrafficBtn{};
  QPushButton* mVirusBtn{};
  QPushButton* mGearBtn{};
};

}  // namespace cse498