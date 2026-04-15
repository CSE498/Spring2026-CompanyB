/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A class that renders the GUI
 *
 * references:
 * https://doc.qt.io/qt-6/qtwidgets-graphicsview-diagramscene-example.html
 * https://doc.qt.io/qt-6/qstatusbar.html
 * https://doc.qt.io/qt-6/qfiledialog.html#getOpenFileName
 * https://doc.qt.io/qt-6/qsplitter.html
 * https://doc.qt.io/qt-6/qkeysequence.html
 *
 **/

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

#include "../../core/WorldBase.hpp"
#include "ImageGrid.hpp"
#include "MainGraph.hpp"

namespace cse498 {

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(WorldBase& world, const std::vector<QString>& imagePaths,
             int tileSize = 32, const QString& agentImagePath = QString(),
             QWidget* parent = nullptr);
  virtual ~MainWindow() = default;
  void logCommand(const QString& message);

  // disable copy
  MainWindow(const MainWindow&) = delete;
  MainWindow& operator=(const MainWindow&) = delete;

  // disable move
  MainWindow(MainWindow&&) = delete;
  MainWindow& operator=(MainWindow&&) = delete;

 private:
  void setMenuBar();
  void setStatusBar();
  void setMainWidget();
  void setImageGrid();

  void onFileNew();
  void onFileOpen();
  void onFileSave();
  void onFileExit();
  void onHelpAbout();

  void startSimulation();
  void onTick();

  // Timer
  QTimer* mTimer = nullptr;
  int mTickInterval = 300;

  // For ImageGrid construction
  WorldBase& mWorld;
  const std::vector<QString>& mImagePaths;
  int mTileSize;
  QString mAgentImagePath;

  // Menu bar
  QMenu* mFileMenu;
  QMenu* mHelpMenu;

  // Actions
  // Raw pointers intentional because of the QT paraent ownership for the menu
  // They get destroyed automatically when the window gets destroyed.
  QAction* mNewFileAction;
  QAction* mOpenFileAction;
  QAction* mSaveFileAction;
  QAction* mExitAction;
  QAction* mAboutAction;

  // Toolbar
  QToolBar* mToolBar;

  // Main widget
  QWidget* mSidePanel;
  QGraphicsView* mGraphicsView;
  QGraphicsScene* mGraphicsScene;

  // ImageGrid
  std::unique_ptr<ImageGrid> mImageGrid;

  // Graph
  MainGraph* mMainGraph;

  // Command Log
  QTextEdit* mCommandLog;
  QVBoxLayout* mSidePanelLayout;
};

}  // namespace cse498