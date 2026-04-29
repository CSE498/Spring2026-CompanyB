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
#include <vector>

#include "../../Worlds/InfectiousWorld.hpp"
#include "MainGraph.hpp"

// based on TrafficMainWindow, adapted for infectious simulation
namespace cse498 {

class InfectiousWindow : public QMainWindow {
  Q_OBJECT

 public:
  InfectiousWindow(InfectiousWorld &world,
                   const std::vector<QString> &imagePaths, int tileSize = 20,
                   const QString &agentImagePath = {},
                   QWidget *parent = nullptr);
  ~InfectiousWindow() override = default;

  // owned world
  // set by the caller (StartScreen) after construction so
  // the window keeps the world alive for its full lifetime
  std::unique_ptr<InfectiousWorld> mOwnedWorld{};

  InfectiousWindow(const InfectiousWindow &) = delete;
  InfectiousWindow &operator=(const InfectiousWindow &) = delete;
  InfectiousWindow(InfectiousWindow &&) = delete;
  InfectiousWindow &operator=(InfectiousWindow &&) = delete;

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

  void onSwitchToInfectiousSimulation();
  void onSwitchToTrafficSimulation();
  void onShowSimulationHelp();

 private:
  void setupMenuBar();
  void setupMainWidget();
  void setupStatusBar();
  void startSimulation();
  void redraw();
  void logCommand(const QString &message);

  InfectiousWorld &mWorld;
  std::vector<QString> mImagePaths;
  int mTileSize;
  QString mAgentImagePath;

  QTimer *mTimer = nullptr;
  int mTickInterval = 150;
  bool mIsRunning = true;
  int mTickCount = 0;

  QGraphicsScene *mGraphicsScene{};
  QGraphicsView *mGraphicsView{};
  QWidget *mSidePanel{};
  MainGraph *mMainGraph{};
  QTextEdit *mCommandLog{};
  QVBoxLayout *mSidePanelLayout{};

  QMenu *mFileMenu{};
  QMenu *mHelpMenu{};
  QToolBar *mToolBar{};

  QAction *mNewFileAction{};
  QAction *mOpenFileAction{};
  QAction *mSaveFileAction{};
  QAction *mExitAction{};
  QAction *mAboutAction{};
  QAction *mBackToMenuAction{};
  QAction *mReplayToggleAction{};
  QAction *mReplayRestartAction{};
};

}  // namespace cse498