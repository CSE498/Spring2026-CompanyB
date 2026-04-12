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
#include <QToolBar>
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
               int tileSize = 32, QWidget* parent = nullptr);
    virtual ~MainWindow() = default;

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

    // For ImageGrid construction
    WorldBase& mWorld;
    const std::vector<QString>& mImagePaths;
    int mTileSize;

    // Menu bar
    QMenu* mFileMenu;
    QMenu* mHelpMenu;

    // Actions
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
};

}  // namespace cse498