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

#include "../../Agents/PacingAgent.hpp"
#include "../../Worlds/InfectiousWorld.hpp"
#include "../../Worlds/MazeWorld.hpp"
#include "../../core/WorldBase.hpp"
#include "ImageGrid.hpp"
#include "MainGraph.hpp"

namespace cse498 {

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(WorldBase& world, const std::vector<QString>& imagePaths,
             int tileSize = 32, const QString& agentImagePath = QString(),
             QWidget* parent = nullptr, int mode = 1);
  virtual ~MainWindow() = default;
  void logCommand(const QString& message);

  // for switching between sims
  void setOwnedWorld(std::unique_ptr<WorldBase> world);

  // disable copy
  MainWindow(const MainWindow&) = delete;
  MainWindow& operator=(const MainWindow&) = delete;

  // disable move
  MainWindow(MainWindow&&) = delete;
  MainWindow& operator=(MainWindow&&) = delete;

signals:
  void requestSimulationSwitch(const QString& simulationName);

 private:
  std::unique_ptr<WorldBase> mOwnedWorld{};
  void setMenuBar();
  void setStatusBar();
  void setMainWidget();
  void setImageGrid();
  void setupAgents();

  void onFileNew();
  void onFileOpen();
  void onFileSave();
  void onFileExit();
  void onSwitchToTrafficSimulation();
  void onSwitchToInfectionSimulation();
  void onReturnToHomeScreen();
  void onHelpAbout();
  void onHelpSimulations();

  void startSimulation();
  void onTick();

  // Replay control
  void onReplayToggle();

  // infection graphs
  void onShowInfectedGraph();
  void onShowSusceptibleGraph();
  void onShowCuredGraph();
  void onShowInfectionBreakdown();

  // traffic graphs
  void onShowWaitingGraph();
  void onShowDrivingGraph();
  void onShowActiveGraph();
  void onShowDistanceGraph();
  void onShowTimeToArriveGraph();
  void onShowCarActivityBreakdown();

  // Timer
  QTimer* mTimer = nullptr;
  int mTickInterval = 150;

  // For ImageGrid construction
  WorldBase& mWorld;
  const std::vector<QString>& mImagePaths;
  int mTileSize;
  QString mAgentImagePath;
  int mMode = 1;

  // Menu bar
  QMenu* mFileMenu;
  QMenu* mMainMenu;
  QMenu* mHelpMenu;
  QMenu* mGraphsMenu;

  QAction* mNewFileAction;
  QAction* mOpenFileAction;
  QAction* mSaveFileAction;
  QAction* mExitAction;

  QAction* mSwitchToTrafficAction;
  QAction* mSwitchToInfectionAction;
  QAction* mReturnHomeAction;
  QAction* mAboutAction;
  QAction* mHelpSimulationsAction;

  // infection graph 
  QAction* mInfectedGraphAction;
  QAction* mSusceptibleGraphAction;
  QAction* mCuredGraphAction;
  QAction* mInfectionBreakdownAction;

  // traffic graph 
  QAction* mWaitingGraphAction;
  QAction* mDrivingGraphAction;
  QAction* mActiveGraphAction;
  QAction* mDistanceGraphAction;
  QAction* mTimeToArriveGraphAction;
  QAction* mCarActivityBreakdownAction;

  QToolBar* mToolBar;

  QWidget* mSidePanel;
  QGraphicsView* mGraphicsView;
  QGraphicsScene* mGraphicsScene;

  std::unique_ptr<ImageGrid> mImageGrid;

  MainGraph* mMainGraph;

  QTextEdit* mCommandLog;
  QVBoxLayout* mSidePanelLayout;

  // Replay control
  QAction* mReplayToggleAction;
  bool mIsRunning = true;

  void onReplayRestart();
  QAction* mReplayRestartAction;
  std::stringstream mInitialState;
};

}  // namespace cse498