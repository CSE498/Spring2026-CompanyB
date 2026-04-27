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

#include "../../Agents/dummyStepPacingAgent.hpp"
#include "../../Worlds/InfectiousWorld.hpp"
#include "../../Worlds/SimWorldBase.hpp"
#include "ImageGrid.hpp"
#include "MainGraph.hpp"

namespace cse498 {

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(SimWorldBase<DiseaseData>& world,
             const std::vector<QString>& imagePaths, int tileSize = 32,
             const QString& agentImagePath = QString(),
             QWidget* parent = nullptr, int mode = 1);
  virtual ~MainWindow() = default;
  void logCommand(const QString& message);


  std::unique_ptr<SimWorldBase<DiseaseData>> mOwnedWorld{};

  // for switching between sims
  void setOwnedWorld(std::unique_ptr<WorldBase> world);

  MainWindow(const MainWindow&) = delete;
  MainWindow& operator=(const MainWindow&) = delete;
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

  void onReplayToggle();

  QTimer* mTimer = nullptr;
  int mTickInterval = 150;

  SimWorldBase<DiseaseData>& mWorld;
  const std::vector<QString>& mImagePaths;
  int mTileSize;
  QString mAgentImagePath;
  int mMode = 1;

  QMenu* mFileMenu;
  QMenu* mMainMenu;
  QMenu* mHelpMenu;

  QAction* mNewFileAction;
  QAction* mOpenFileAction;
  QAction* mSaveFileAction;
  QAction* mExitAction;

  QAction* mSwitchToTrafficAction;
  QAction* mSwitchToInfectionAction;
  QAction* mReturnHomeAction;
  QAction* mAboutAction;
  QAction* mHelpSimulationsAction;

  QToolBar* mToolBar;

  QWidget* mSidePanel;
  QGraphicsView* mGraphicsView;
  QGraphicsScene* mGraphicsScene;

  std::unique_ptr<ImageGrid> mImageGrid;

  MainGraph* mMainGraph;

  QTextEdit* mCommandLog;
  QVBoxLayout* mSidePanelLayout;

  QAction* mReplayToggleAction;
  bool mIsRunning = true;

  void onReplayRestart();
  QAction* mReplayRestartAction;
  std::stringstream mInitialState;
};

}  // namespace cse498