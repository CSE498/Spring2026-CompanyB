#pragma once

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <memory>

#include "../../Agents/ScriptedAgent.hpp"
#include "../../Agents/SwarmingAgent.hpp"
#include "../../Worlds/InfectiousWorld.hpp"
#include "../../Worlds/SimWorldBase.hpp"
#include "../../Worlds/StepTrafficWorld.hpp"
#include "../../tools/Box.hpp"
#include "../../tools/Point.hpp"
#include "InfectiousWindow.hpp"
#include "MainWindow.hpp"
#include "TrafficMainWindow.hpp"

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
  std::unique_ptr<InfectiousWorld> mDiseaseWorld{};
  std::unique_ptr<StepTrafficWorld<SwarmingAgent<TrafficData>>> mTrafficWorld{};

  QPushButton* mTrafficBtn{};
  QPushButton* mVirusBtn{};
  QPushButton* mGearBtn{};
};

}  // namespace cse498