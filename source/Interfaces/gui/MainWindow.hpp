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
#include <QTimer>
#include <QTextEdit>
#include <QVBoxLayout>
#include <memory>
#include <vector>

#include "../../core/WorldBase.hpp"
#include "../../Worlds/InfectiousWorld.hpp"
#include "../../Worlds/MazeWorld.hpp"
#include "../../Agents/PacingAgent.hpp"
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

    std::unique_ptr<WorldBase> mOwnedWorld{};

    // disable copy
    MainWindow(const MainWindow &) = delete;
    MainWindow &operator=(const MainWindow &) = delete;

    // disable move
    MainWindow(MainWindow &&) = delete;
    MainWindow &operator=(MainWindow &&) = delete;

private:
    void setMenuBar();
    void setStatusBar();
    void setMainWidget();
    void setImageGrid();
    void setupAgents();

    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileExit();
    void onHelpAbout();
    void onBackToMainMenu();

    void startSimulation();
    void onTick();

    // Replay control
    void onReplayToggle();

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
    QMenu* mHelpMenu;

    QAction* mNewFileAction;
    QAction* mOpenFileAction;
    QAction* mSaveFileAction;
    QAction* mExitAction;
    QAction* mAboutAction;
    QAction* mBackToMenuAction;

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
};

} // namespace cse498