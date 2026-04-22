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
#include <sstream>

#include "../../Worlds/SimWorldBase.hpp"
#include "../../Worlds/InfectiousWorld.hpp"
#include "../../Agents/dummyStepPacingAgent.hpp"
#include "ImageGrid.hpp"
#include "MainGraph.hpp"

namespace cse498 {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(SimWorldBase<DiseaseData>& world, const std::vector<QString>& imagePaths,
               int tileSize = 32, const QString& agentImagePath = QString(),
               QWidget* parent = nullptr, int mode = 1);
    virtual ~MainWindow() = default;
    void logCommand(const QString& message);

    std::unique_ptr<SimWorldBase<DiseaseData>> mOwnedWorld{};

    MainWindow(const MainWindow &) = delete;
    MainWindow &operator=(const MainWindow &) = delete;
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

    void onReplayToggle();

    QTimer* mTimer = nullptr;
    int mTickInterval = 150;

    SimWorldBase<DiseaseData>& mWorld;
    const std::vector<QString>& mImagePaths;
    int mTileSize;
    QString mAgentImagePath;
    int mMode = 1;

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

    QAction* mReplayToggleAction;
    bool mIsRunning = true;

    void onReplayRestart();
    QAction* mReplayRestartAction;
    std::stringstream mInitialState;
};

} // namespace cse498