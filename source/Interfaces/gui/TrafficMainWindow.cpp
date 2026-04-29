#include "TrafficMainWindow.hpp"

#include <QApplication>
#include <QBrush>
#include <QFileDialog>
#include <QMessageBox>
#include <QPen>
#include <QSizePolicy>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>
#include <fstream>
#include <string>
#include <vector>

#include "StartScreen.h"

namespace cse498 {

constexpr int TRAFFIC_TIMEOUT = 4000;

TrafficMainWindow::TrafficMainWindow(TrafficWorld& world,
                                     const std::vector<QString>& imagePaths,
                                     int tileSize,
                                     const QString& agentImagePath,
                                     QWidget* parent)
    : QMainWindow(parent),
      mWorld(world),
      mImagePaths(imagePaths),
      mTileSize(tileSize),
      mAgentImagePath(agentImagePath) {
  setWindowTitle("Group 21 Demo - Traffic");
  setMinimumSize(640, 480);
  resize(1280, 720);

  setMenuBar();
  setMainWidget();
  setStatusBar();
  startSimulation();
}

void TrafficMainWindow::setMenuBar() {
  mFileMenu = menuBar()->addMenu("&File");

  mNewFileAction =
      new QAction(QIcon::fromTheme("document-new"), "&New File...", this);
  mNewFileAction->setShortcut(QKeySequence::New);
  mNewFileAction->setStatusTip("Create a new file");
  connect(mNewFileAction, &QAction::triggered, this,
          &TrafficMainWindow::onFileNew);

  mOpenFileAction =
      new QAction(QIcon::fromTheme("document-open"), "&Open File...", this);
  mOpenFileAction->setShortcut(QKeySequence::Open);
  mOpenFileAction->setStatusTip("Open an existing file");
  connect(mOpenFileAction, &QAction::triggered, this,
          &TrafficMainWindow::onFileOpen);

  mSaveFileAction =
      new QAction(QIcon::fromTheme("document-save"), "&Save", this);
  mSaveFileAction->setShortcut(QKeySequence::Save);
  mSaveFileAction->setStatusTip("Save the current file");
  connect(mSaveFileAction, &QAction::triggered, this,
          &TrafficMainWindow::onFileSave);

  mBackToMenuAction =
      new QAction(QIcon::fromTheme("go-home"), "&Back to Main Menu", this);
  mBackToMenuAction->setStatusTip("Return to the main menu");
  connect(mBackToMenuAction, &QAction::triggered, this,
          &TrafficMainWindow::onBackToMainMenu);

  mExitAction =
      new QAction(QIcon::fromTheme("application-exit"), "E&xit", this);
  mExitAction->setShortcut(QKeySequence::Quit);
  mExitAction->setStatusTip("Exit the application");
  connect(mExitAction, &QAction::triggered, this,
          &TrafficMainWindow::onFileExit);

  mFileMenu->addAction(mNewFileAction);
  mFileMenu->addAction(mOpenFileAction);
  mFileMenu->addAction(mSaveFileAction);
  mFileMenu->addSeparator();
  mFileMenu->addAction(mExitAction);

  mHelpMenu = menuBar()->addMenu("&Help");

  mSimulationHelpAction = new QAction("&Current Simulation Help", this);
  mSimulationHelpAction->setStatusTip("Explain the current simulation");
  connect(mSimulationHelpAction, &QAction::triggered, this,
          &TrafficMainWindow::onShowSimulationHelp);

  mAboutAction = new QAction(QIcon::fromTheme("help-about"), "&About", this);
  mAboutAction->setStatusTip("Show information about this application");
  connect(mAboutAction, &QAction::triggered, this,
          &TrafficMainWindow::onHelpAbout);

  mHelpMenu->addAction(mSimulationHelpAction);
  mHelpMenu->addSeparator();
  mHelpMenu->addAction(mAboutAction);

  mMainMenu = menuBar()->addMenu("&Main Menu");

  mSwitchToTrafficAction = new QAction("&Switch to Traffic Simulation", this);
  mSwitchToTrafficAction->setStatusTip(
      "You are already in the traffic simulation");
  mSwitchToTrafficAction->setEnabled(false);

  mSwitchToVirusAction = new QAction("&Switch to Virus Simulation", this);
  mSwitchToVirusAction->setStatusTip("Open the virus simulation");
  connect(mSwitchToVirusAction, &QAction::triggered, this,
          &TrafficMainWindow::onSwitchToVirusSimulation);

  mReturnToStartAction = new QAction("&Return to Start Screen", this);
  mReturnToStartAction->setStatusTip("Return to the main start screen");
  connect(mReturnToStartAction, &QAction::triggered, this,
          &TrafficMainWindow::onBackToMainMenu);

  mMainMenu->addAction(mSwitchToTrafficAction);
  mMainMenu->addAction(mSwitchToVirusAction);
  mMainMenu->addSeparator();
  mMainMenu->addAction(mReturnToStartAction);

  mToolBar = addToolBar("Simulation");
  mToolBar->setMovable(false);

  mReplayToggleAction = new QAction("Pause", this);
  mReplayToggleAction->setShortcut(QKeySequence(Qt::Key_Space));
  mReplayToggleAction->setStatusTip("Pause or resume the simulation");
  connect(mReplayToggleAction, &QAction::triggered, this,
          &TrafficMainWindow::onReplayToggle);
  mToolBar->addAction(mReplayToggleAction);

  mReplayRestartAction = new QAction("Restart", this);
  mReplayRestartAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
  mReplayRestartAction->setStatusTip("Reset and restart the simulation");
  connect(mReplayRestartAction, &QAction::triggered, this,
          &TrafficMainWindow::onReplayRestart);
  mToolBar->addAction(mReplayRestartAction);
}

void TrafficMainWindow::setMainWidget() {
  mGraphicsScene = new QGraphicsScene(this);
  mGraphicsView = new QGraphicsView(mGraphicsScene, this);
  mGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

  mSidePanel = new QWidget(this);
  mSidePanel->setMinimumWidth(200);

  auto* verticalSplitter = new QSplitter(Qt::Vertical, mSidePanel);

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

  auto* splitter = new QSplitter(Qt::Horizontal, this);
  splitter->addWidget(mSidePanel);
  splitter->addWidget(mGraphicsView);
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 3);
  setCentralWidget(splitter);
}

void TrafficMainWindow::setStatusBar() {
  statusBar()->showMessage("This is a status bar!", TRAFFIC_TIMEOUT);
}

void TrafficMainWindow::startSimulation() {
  mTimer = new QTimer(this);
  connect(mTimer, &QTimer::timeout, this, &TrafficMainWindow::onTick);
  mTimer->start(mTickInterval);
  redraw();
}

void TrafficMainWindow::onTick() {
  mWorld.RunAgents();
  mWorld.UpdateWorld();
  ++mTickCount;
  redraw();

  if (mTickCount % 10 == 0) {
    int active = 0;
    for (size_t i = 0; i < mWorld.GetNumAgents(); ++i)
      if (mWorld.GetAgentState(i).is_active) ++active;
    logCommand(
        QString("[Tick %1] Active agents: %2").arg(mTickCount).arg(active));
  }
}

void TrafficMainWindow::redraw() {
  mGraphicsScene->clear();
  const WorldGrid& grid = mWorld.GetGrid();
  const int T = mTileSize;
  const size_t W = grid.GetWidth();
  const size_t H = grid.GetHeight();

  for (size_t y = 0; y < H; ++y) {
    for (size_t x = 0; x < W; ++x) {
      WorldPosition pos{x, y};
      QColor colour;
      if (mWorld.IsGrass(pos))
        colour = QColor(34, 85, 34);
      else if (mWorld.HorizontalBlockedAt(pos))
        colour = QColor(180, 50, 50);
      else if (mWorld.VerticalBlockedAt(pos))
        colour = QColor(50, 50, 180);
      else
        colour = QColor(80, 80, 80);

      mGraphicsScene->addRect(static_cast<int>(x) * T, static_cast<int>(y) * T,
                              T, T, QPen(Qt::NoPen), QBrush(colour));
    }
  }

  for (size_t i = 0; i < mWorld.GetNumAgents(); ++i) {
    TrafficData state = mWorld.GetAgentState(i);
    if (!state.is_active) continue;
    const int ax = static_cast<int>(state.position.CellX()) * T;
    const int ay = static_cast<int>(state.position.CellY()) * T;
    mGraphicsScene->addEllipse(ax + 4, ay + 4, T - 8, T - 8, QPen(Qt::NoPen),
                               QBrush(QColor(255, 200, 0)));
  }
}

void TrafficMainWindow::logCommand(const QString& message) {
  mCommandLog->append(message);
}

void TrafficMainWindow::onFileNew() {
  statusBar()->showMessage("New file created", TRAFFIC_TIMEOUT);
}

void TrafficMainWindow::onFileOpen() {
  const QString path = QFileDialog::getOpenFileName(
      this, "Open File", QString(), "Grid Files (*.grid);;All Files (*.*)");
  if (path.isEmpty()) return;
  statusBar()->showMessage(QString("Opened: %1").arg(path), TRAFFIC_TIMEOUT);
}

void TrafficMainWindow::onFileSave() {
  const QString path =
      QFileDialog::getSaveFileName(this, "Save File", QString(),
                                   "Text Files (*.txt *.sim);;All Files (*.*)");
  if (path.isEmpty()) return;

  std::ofstream output(path.toStdString());
  if (!output.is_open()) {
    QMessageBox::warning(this, "Save Failed", "Could not save file.");
    return;
  }

  output << "--- COMMAND LOG ---\n";
  output << mCommandLog->toPlainText().toStdString() << "\n";
  output << "--- GRAPH DATA ---\n";
  output << "tick=" << mTickCount << "\n";

  statusBar()->showMessage(QString("File Saved: %1").arg(path),
                           TRAFFIC_TIMEOUT);
}

void TrafficMainWindow::onFileExit() { close(); }

void TrafficMainWindow::onHelpAbout() {
  QMessageBox::about(this, "About", "<b>Group 21 Demo</b>");
}

void TrafficMainWindow::onBackToMainMenu() {
  if (mTimer) {
    mTimer->stop();
  }

  auto* startScreen = new StartScreen(mImagePaths, mTileSize, mAgentImagePath);
  startScreen->setAttribute(Qt::WA_DeleteOnClose);
  startScreen->show();

  hide();
  deleteLater();
}

void TrafficMainWindow::onReplayToggle() {
  if (mIsRunning) {
    mTimer->stop();
    mReplayToggleAction->setText("Play");
    statusBar()->showMessage("Simulation paused", TRAFFIC_TIMEOUT);
    logCommand("[System] Simulation paused.");
  } else {
    mTimer->start(mTickInterval);
    mReplayToggleAction->setText("Pause");
    statusBar()->showMessage("Simulation resumed", TRAFFIC_TIMEOUT);
    logCommand("[System] Simulation resumed.");
  }
  mIsRunning = !mIsRunning;
}

void TrafficMainWindow::onReplayRestart() {
  mTimer->stop();
  mTickCount = 0;
  mWorld.ClearAgents();

  redraw();
  mCommandLog->clear();
  mIsRunning = true;
  mReplayToggleAction->setText("Pause");
  mTimer->start(mTickInterval);
  statusBar()->showMessage("Simulation restarted", TRAFFIC_TIMEOUT);
  logCommand("[System] Simulation restarted.");
}

void TrafficMainWindow::onShowSimulationHelp() {
  QMessageBox::information(
      this, "Traffic Simulation Help",
      "Current Simulation: Traffic Simulation\n\n"
      "This simulation shows agents moving through a traffic-style grid. "
      "Different colors represent different terrain or blocked directions, "
      "and active agents are drawn as moving circles.\n\n"
      "Use Pause to stop the simulation, Restart to reset it, and Main Menu "
      "to switch simulations or return to the start screen.");
}

void TrafficMainWindow::onSwitchToTrafficSimulation() {
  statusBar()->showMessage("Already in Traffic Simulation", TRAFFIC_TIMEOUT);
}

void TrafficMainWindow::onSwitchToVirusSimulation() {
  if (mTimer) {
    mTimer->stop();
  }

  // Create the infection/virus world the same way StartScreen creates it.
  // This example assumes InfectiousWorld has a default constructor.
  auto* virusWorld = new InfectiousWorld();

  auto* virusWindow = new MainWindow(*virusWorld, mImagePaths, mTileSize,
                                     mAgentImagePath, nullptr, 2);

  virusWindow->setAttribute(Qt::WA_DeleteOnClose);
  virusWindow->show();

  close();
}

}  // namespace cse498