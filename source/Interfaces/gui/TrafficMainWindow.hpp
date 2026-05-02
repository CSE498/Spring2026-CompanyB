#pragma once

#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QString>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <sstream>
#include <vector>

#include "../../Agents/SwarmingAgent.hpp"
#include "../../Worlds/StepTrafficWorld.hpp"
#include "../../tools/DataLog.hpp"
#include "MainGraph.hpp"

namespace cse498 {

using TrafficWorld = StepTrafficWorld<SwarmingAgent<TrafficData>>;

class TrafficMainWindow : public QMainWindow {
  Q_OBJECT
 public:
  TrafficMainWindow(TrafficWorld& world, const std::vector<QString>& imagePaths,
                    int tileSize = 32, const QString& agentImagePath = {},
                    QWidget* parent = nullptr);
  ~TrafficMainWindow() override = default;

  std::unique_ptr<TrafficWorld> mOwnedWorld{};

  TrafficMainWindow(const TrafficMainWindow&) = delete;
  TrafficMainWindow& operator=(const TrafficMainWindow&) = delete;
  TrafficMainWindow(TrafficMainWindow&&) = delete;
  TrafficMainWindow& operator=(TrafficMainWindow&&) = delete;

 private slots:
  void onTick();
  void onReplayToggle();
  void onReplayRestart();
  void onBackToMainMenu();
  void onFileNew();
  void onFileOpen();
  void onFileSave();
  void onFileExit();
  void onHelpAbout();

  void onSwitchToTrafficSimulation();
  void onSwitchToVirusSimulation();
  void onShowSimulationHelp();

  void onShowWaitingGraph();
  void onShowDrivingGraph();
  void onShowActiveGraph();
  void onShowDistanceGraph();
  void onShowTimeToArriveGraph();
  void onShowCarActivityBreakdown();

 private:
  void setMenuBar();
  void setStatusBar();
  void setMainWidget();
  void startSimulation();
  void redraw();
  void logCommand(const QString& message);

  TrafficWorld& mWorld;
  std::vector<QString> mImagePaths;
  int mTileSize;
  QString mAgentImagePath;

  QTimer* mTimer = nullptr;
  const int mTickInterval = 150;
  bool mIsRunning = true;
  int mTickCount = 0;

  QGraphicsScene* mGraphicsScene{};
  QGraphicsView* mGraphicsView{};
  QWidget* mSidePanel{};
  MainGraph* mMainGraph{};
  QTextEdit* mCommandLog{};
  QVBoxLayout* mSidePanelLayout{};

  QMenu* mFileMenu{};
  QMenu* mHelpMenu{};
  QToolBar* mToolBar{};
  QMenu* mMainMenu = nullptr;

  QMenu* mGraphsMenu{};
  QAction* mWaitingGraphAction{};
  QAction* mDrivingGraphAction{};
  QAction* mActiveGraphAction{};
  QAction* mDistanceGraphAction{};
  QAction* mTimeToArriveGraphAction{};
  QAction* mCarActivityBreakdownAction{};

  QAction* mSwitchToTrafficAction = nullptr;
  QAction* mSwitchToVirusAction = nullptr;
  QAction* mReturnToStartAction = nullptr;
  QAction* mSimulationHelpAction = nullptr;

  QAction* mNewFileAction{};
  QAction* mOpenFileAction{};
  QAction* mSaveFileAction{};
  QAction* mExitAction{};
  QAction* mAboutAction{};
  QAction* mBackToMenuAction{};
  QAction* mReplayToggleAction{};
  QAction* mReplayRestartAction{};
};

}  // namespace cse498