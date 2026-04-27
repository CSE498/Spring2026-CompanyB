#include "MainWindow.hpp"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QSizePolicy>
#include <QSplitter>
#include <QVBoxLayout>
#include <fstream>
#include <string>
#include <vector>

#include "StartScreen.h"

namespace cse498 {

constexpr const int TIMEOUT = 4000;

MainWindow::MainWindow(WorldBase &world, const std::vector<QString> &imagePaths,
                       int tileSize, const QString &agentImagePath,
                       QWidget *parent, int mode)
    : QMainWindow(parent),
      mWorld(world),
      mImagePaths(imagePaths),
      mTileSize(tileSize),
      mAgentImagePath(agentImagePath),
      mMode(mode) {
  setWindowTitle("Group 21 Demo");
  setMinimumSize(640, 480);
  resize(1280, 720);

  setMenuBar();
  setMainWidget();
  setupAgents();
  setImageGrid();
  setStatusBar();

  mWorld.GetGrid().Print(mInitialState);

  startSimulation();
}

void MainWindow::setupAgents() {
  if (mMode == 1) {
  } else {
    auto &iw = static_cast<InfectiousWorld &>(mWorld);
    iw.AddAgent<PacingAgent>("Agent-1").SetLocation(WorldPosition{3, 2});
    iw.AddAgent<PacingAgent>("Agent-2").SetLocation(WorldPosition{5, 4});
    iw.AddAgent<PacingAgent>("Agent-3").SetLocation(WorldPosition{7, 3});
    iw.AddAgent<PacingAgent>("Agent-4").SetLocation(WorldPosition{4, 6});
    iw.AddAgent<PacingAgent>("Agent-5").SetLocation(WorldPosition{8, 7});
    iw.AddAgent<PacingAgent>("Agent-6").SetLocation(WorldPosition{12, 3});
    iw.AddAgent<PacingAgent>("Agent-7").SetLocation(WorldPosition{15, 5});
    iw.AddAgent<PacingAgent>("Agent-8").SetLocation(WorldPosition{14, 2});
    iw.AddAgent<PacingAgent>("Agent-9").SetLocation(WorldPosition{16, 7});
    iw.AddAgent<PacingAgent>("Agent-10").SetLocation(WorldPosition{13, 7});
    iw.InfectAgent(0);
  }
}

void MainWindow::startSimulation() {
  mTimer = new QTimer(this);
  connect(mTimer, &QTimer::timeout, this, &MainWindow::onTick);
  mTimer->start(mTickInterval);
}

void MainWindow::onTick() {
  mWorld.RunAgents();
  mWorld.UpdateWorld();
  mImageGrid->RenderGrid();
  mImageGrid->RenderAgents();
  logCommand("TEST Tick 1: [Agent 0] Move forward TEST");
  logCommand("TEST Tick 2: [Agent 1] Stop TEST");
}

void MainWindow::setMenuBar() {
  mFileMenu = menuBar()->addMenu("&File");

  mNewFileAction =
      new QAction(QIcon::fromTheme("document-new"), ("&New File..."), this);
  mNewFileAction->setShortcut(QKeySequence::New);
  mNewFileAction->setStatusTip("Create a new file");
  connect(mNewFileAction, &QAction::triggered, this, &MainWindow::onFileNew);

  mOpenFileAction =
      new QAction(QIcon::fromTheme("document-open"), ("&Open File..."), this);
  mOpenFileAction->setShortcut(QKeySequence::Open);
  mOpenFileAction->setStatusTip("Open an existing file");
  connect(mOpenFileAction, &QAction::triggered, this, &MainWindow::onFileOpen);

  mSaveFileAction =
      new QAction(QIcon::fromTheme("document-save"), ("&Save"), this);
  mSaveFileAction->setShortcut(QKeySequence::Save);
  mSaveFileAction->setStatusTip("Save the current file");
  connect(mSaveFileAction, &QAction::triggered, this, &MainWindow::onFileSave);

  mBackToMenuAction =
      new QAction(QIcon::fromTheme("go-home"), ("&Back to Main Menu"), this);
  mBackToMenuAction->setStatusTip("Return to the main menu");
  connect(mBackToMenuAction, &QAction::triggered, this,
          &MainWindow::onBackToMainMenu);

  mExitAction =
      new QAction(QIcon::fromTheme("application-exit"), ("E&xit"), this);
  mExitAction->setShortcut(QKeySequence::Quit);
  mExitAction->setStatusTip("Exit the application");
  connect(mExitAction, &QAction::triggered, this, &MainWindow::onFileExit);

  mFileMenu->addAction(mNewFileAction);
  mFileMenu->addAction(mOpenFileAction);
  mFileMenu->addAction(mSaveFileAction);
  mFileMenu->addSeparator();
  mFileMenu->addAction(mBackToMenuAction);
  mFileMenu->addSeparator();
  mFileMenu->addAction(mExitAction);

  mHelpMenu = menuBar()->addMenu("&Help");
  mAboutAction = new QAction(QIcon::fromTheme("help-about"), ("&About"), this);
  mAboutAction->setStatusTip("Show information about this application");
  connect(mAboutAction, &QAction::triggered, this, &MainWindow::onHelpAbout);
  mHelpMenu->addAction(mAboutAction);

  // Toolbar
  mToolBar = addToolBar("Simulation");
  mToolBar->setMovable(false);

  mReplayToggleAction = new QAction("⏸ Pause", this);
  mReplayToggleAction->setShortcut(QKeySequence(Qt::Key_Space));
  mReplayToggleAction->setStatusTip("Pause or resume the simulation");
  connect(mReplayToggleAction, &QAction::triggered, this,
          &MainWindow::onReplayToggle);
  mToolBar->addAction(mReplayToggleAction);

  mReplayRestartAction = new QAction("⏮ Restart", this);
  mReplayRestartAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
  mReplayRestartAction->setStatusTip("Reset and restart the simulation");
  connect(mReplayRestartAction, &QAction::triggered, this,
          &MainWindow::onReplayRestart);
  mToolBar->addAction(mReplayRestartAction);

  // Graphs menu
  mGraphsMenu = menuBar()->addMenu("&Graphs");

  if (mMode == 2) {
    mInfectedGraphAction = new QAction("Infected Count Over Time", this);
    mSusceptibleGraphAction = new QAction("Susceptible Count Over Time", this);
    mCuredGraphAction = new QAction("Cured Count Over Time", this);
    mInfectionBreakdownAction = new QAction("Current Infection Breakdown", this);

    mGraphsMenu->addAction(mInfectedGraphAction);
    mGraphsMenu->addAction(mSusceptibleGraphAction);
    mGraphsMenu->addAction(mCuredGraphAction);
    mGraphsMenu->addAction(mInfectionBreakdownAction);

    connect(mInfectedGraphAction, &QAction::triggered, this, &MainWindow::onShowInfectedGraph);
    connect(mSusceptibleGraphAction, &QAction::triggered, this, &MainWindow::onShowSusceptibleGraph);
    connect(mCuredGraphAction, &QAction::triggered, this, &MainWindow::onShowCuredGraph);
    connect(mInfectionBreakdownAction, &QAction::triggered, this, &MainWindow::onShowInfectionBreakdown);
  }

  if (mMode == 1) {
    mWaitingGraphAction = new QAction("Waiting Count Over Time", this);
    mDrivingGraphAction = new QAction("Driving Count Over Time", this);
    mActiveGraphAction = new QAction("Active Count Over Time", this);
    mDistanceGraphAction = new QAction("Distance Driven Over Time", this);
    mTimeToArriveGraphAction = new QAction("Time to Arrive Over Time", this);
    mCarActivityBreakdownAction = new QAction("Current Car Activity Breakdown", this);

    mGraphsMenu->addAction(mWaitingGraphAction);
    mGraphsMenu->addAction(mDrivingGraphAction);
    mGraphsMenu->addAction(mActiveGraphAction);
    mGraphsMenu->addAction(mDistanceGraphAction);
    mGraphsMenu->addAction(mTimeToArriveGraphAction);
    mGraphsMenu->addAction(mCarActivityBreakdownAction);

    connect(mWaitingGraphAction, &QAction::triggered, this, &MainWindow::onShowWaitingGraph);
    connect(mDrivingGraphAction, &QAction::triggered, this, &MainWindow::onShowDrivingGraph);
    connect(mActiveGraphAction, &QAction::triggered, this, &MainWindow::onShowActiveGraph);
    connect(mDistanceGraphAction, &QAction::triggered, this, &MainWindow::onShowDistanceGraph);
    connect(mTimeToArriveGraphAction, &QAction::triggered, this, &MainWindow::onShowTimeToArriveGraph);
    connect(mCarActivityBreakdownAction, &QAction::triggered, this, &MainWindow::onShowCarActivityBreakdown);
  }
}

void MainWindow::setMainWidget() {
  mGraphicsScene = new QGraphicsScene(this);
  mGraphicsView = new QGraphicsView(mGraphicsScene, this);
  mGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

  mSidePanel = new QWidget(this);
  mSidePanel->setMinimumWidth(200);

  auto *verticalSplitter = new QSplitter(Qt::Vertical, mSidePanel);

  mMainGraph = new MainGraph(verticalSplitter);
  verticalSplitter->addWidget(mMainGraph);

  mCommandLog = new QTextEdit(verticalSplitter);
  mCommandLog->setReadOnly(true);
  mCommandLog->setStyleSheet(
      "QTextEdit {"
      "  background-color: #4a6741;"
      "  color: #e0e0e0;"
      "  border: 2px solid #2d4a2d;"
      "  border-radius: 8px;"
      "  padding: 8px;"
      "  font-family: monospace;"
      "  font-size: 12px;"
      "}");
  mCommandLog->setPlaceholderText("Commands will appear here...");
  verticalSplitter->addWidget(mCommandLog);
  mSidePanelLayout = new QVBoxLayout(mSidePanel);
  mSidePanelLayout->addWidget(verticalSplitter);
  mSidePanel->setLayout(mSidePanelLayout);

  auto *splitter = new QSplitter(Qt::Horizontal, this);
  splitter->addWidget(mSidePanel);
  splitter->addWidget(mGraphicsView);
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 3);
  setCentralWidget(splitter);

  if (mMode == 1) {
    onShowWaitingGraph();
  } else {
    onShowInfectedGraph();
  }
}

void MainWindow::setImageGrid() {
  mImageGrid = std::make_unique<ImageGrid>(mWorld, *mGraphicsScene, mTileSize);
  mImageGrid->MapImages(mImagePaths);
  mImageGrid->LoadAgentImage(mAgentImagePath);
  mImageGrid->RenderGrid();
  mImageGrid->RenderAgents();
  mImageGrid->SetSceneAndView(*mGraphicsView);
}

void MainWindow::setStatusBar() {
  statusBar()->showMessage("This is a status bar!", TIMEOUT);
}

void MainWindow::onFileNew() {
  statusBar()->showMessage("New file created", TIMEOUT);
}

void MainWindow::onFileOpen() {
  const QString path = QFileDialog::getOpenFileName(
      this, "Open File", QString(),
      "Text Files (*.txt *.al *.sim);;All Files (*.*)");
  if (path.isEmpty()) return;

  std::ifstream input(path.toStdString());
  if (!input.is_open()) {
    QMessageBox::warning(this, "Open Failed", "Could not open file.");
    return;
  }

  std::vector<std::string> worldLines;
  QStringList logLines;
  std::vector<std::string> graphLines;

  enum Section { NONE, WORLD, LOG, GRAPH };
  Section current = NONE;

  std::string line;
  while (std::getline(input, line)) {
    if (line == "--- WORLD ---") {
      current = WORLD;
      continue;
    }
    if (line == "--- COMMAND LOG ---") {
      current = LOG;
      continue;
    }
    if (line == "--- GRAPH DATA ---") {
      current = GRAPH;
      continue;
    }

    switch (current) {
      case WORLD:
        if (!line.empty()) worldLines.push_back(line);
        break;
      case LOG:
        logLines << QString::fromStdString(line);
        break;
      case GRAPH:
        graphLines.push_back(line);
        break;
      default:
        break;
    }
  }

  if (!worldLines.empty()) {
    mWorld.GetGrid().Load(worldLines);
    mGraphicsScene->clear();
    setImageGrid();
  }

  mCommandLog->setPlainText(logLines.join("\n"));

  // parse graphLines here and update mMainGraph

  statusBar()->showMessage(QString("Opened: %1").arg(path), TIMEOUT);
}

void MainWindow::onFileSave() {
  const QString path = QFileDialog::getSaveFileName(
      this, "Save File", QString(),
      "Text Files (*.txt *.al *.sim);;All Files (*.*)");
  if (path.isEmpty()) return;

  std::ofstream output(path.toStdString());
  if (!output.is_open()) {
    QMessageBox::warning(this, "Save Failed", "Could not save file.");
    return;
  }

  output << "--- WORLD ---\n";
  mWorld.GetGrid().Print(output);

  output << "--- COMMAND LOG ---\n";
  output << mCommandLog->toPlainText().toStdString() << "\n";

  output << "--- GRAPH DATA ---\n";
  output << "tick=" << 123 << "\n";
  output << "mode=" << mMode << "\n";

  statusBar()->showMessage(QString("File Saved: %1").arg(path), TIMEOUT);
}

void MainWindow::onFileExit() { close(); }

void MainWindow::onHelpAbout() {
  QMessageBox::about(this, "About", "<b>Group 21 Demo</b>");
}

void MainWindow::onBackToMainMenu() {
  if (mTimer) {
    mTimer->stop();
  }

  auto *startScreen = new StartScreen(mImagePaths, mTileSize, mAgentImagePath);
  startScreen->setAttribute(Qt::WA_DeleteOnClose);
  startScreen->show();

  close();
}

void MainWindow::logCommand(const QString &message) {
  mCommandLog->append(message);
}

void MainWindow::onReplayToggle() {
  if (mIsRunning) {
    mTimer->stop();
    mReplayToggleAction->setText("▶ Play");
    statusBar()->showMessage("Simulation paused", TIMEOUT);
    logCommand("[System] Simulation paused.");
  } else {
    mTimer->start(mTickInterval);
    mReplayToggleAction->setText("⏸ Pause");
    statusBar()->showMessage("Simulation resumed", TIMEOUT);
    logCommand("[System] Simulation resumed.");
  }
  mIsRunning = !mIsRunning;
}

void MainWindow::onReplayRestart() {
  mTimer->stop();

  std::istringstream input(mInitialState.str());
  mWorld.GetGrid().Load(input);

  setupAgents();

  mGraphicsScene->clear();
  setImageGrid();

  mCommandLog->clear();

  mIsRunning = true;
  mReplayToggleAction->setText("⏸ Pause");
  mTimer->start(mTickInterval);

  statusBar()->showMessage("Simulation restarted", TIMEOUT);
  logCommand("[System] Simulation restarted.");
}

// infection graphs
void MainWindow::onShowInfectedGraph() {
    // TODO: replace dummy data with real data 

    std::vector<double> dummyData = {1, 3, 5, 4, 6, 8, 7, 9};
    mMainGraph->ShowLineGraph("Infected Count Over Time", "Infected", dummyData);
}

void MainWindow::onShowSusceptibleGraph() {
    // TODO: replace dummy data with real data o

    std::vector<double> dummyData = {10, 8, 6, 7, 5, 3, 4, 2};
    mMainGraph->ShowLineGraph("Susceptible Count Over Time", "Susceptible", dummyData);
}

void MainWindow::onShowCuredGraph() {
    // TODO: replace dummy data with real data 
    std::vector<double> dummyData = {0, 1, 1, 2, 3, 4, 4, 5};
    mMainGraph->ShowLineGraph("Cured Count Over Time", "Cured", dummyData);
}

void MainWindow::onShowInfectionBreakdown() {
    // TODO: replace dummy data with real data 

    std::vector<std::pair<QString, double>> slices = {
        {"Infected", 5.0},
        {"Susceptible", 3.0},
        {"Cured", 2.0}
    };
    mMainGraph->ShowPieChart("Current Infection Breakdown", slices);
}

// traffic graphs
void MainWindow::onShowWaitingGraph() {
    // TODO: replace dummy data with real data 

    std::vector<double> dummyData = {5, 4, 6, 3, 7, 5, 4, 6};
    mMainGraph->ShowLineGraph("Waiting Count Over Time", "Waiting", dummyData);
}

void MainWindow::onShowDrivingGraph() {
    // TODO: replace dummy data with real data 

    std::vector<double> dummyData = {3, 5, 4, 7, 6, 8, 7, 9};
    mMainGraph->ShowLineGraph("Driving Count Over Time", "Driving", dummyData);
}

void MainWindow::onShowActiveGraph() {
    // TODO: replace dummy data with real data 

    std::vector<double> dummyData = {8, 9, 10, 10, 13, 13, 11, 15};
    mMainGraph->ShowLineGraph("Active Count Over Time", "Active", dummyData);
}

void MainWindow::onShowDistanceGraph() {
    // TODO: replace dummy data with real data 

    std::vector<double> dummyData = {2, 5, 8, 12, 17, 23, 30, 38};
    mMainGraph->ShowLineGraph("Distance Driven Over Time", "Distance", dummyData);
}

void MainWindow::onShowTimeToArriveGraph() {
    // TODO: replace dummy data with real data once 


    std::vector<double> dummyData = {10, 9, 8, 7, 7, 6, 5, 4};
    mMainGraph->ShowLineGraph("Time to Arrive Over Time", "Time", dummyData);
}

void MainWindow::onShowCarActivityBreakdown() {
    // TODO: replace dummy data with real data 


    std::vector<std::pair<QString, double>> slices = {
        {"Waiting", 4.0},
        {"Driving", 7.0},
        {"Active", 3.0}
    };
    mMainGraph->ShowPieChart("Current Car Activity Breakdown", slices);
}

}  // namespace cse498