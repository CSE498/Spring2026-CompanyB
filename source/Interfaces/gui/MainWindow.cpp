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
#include <QMessageBox>
#include <QPixmap>

#include "StartScreen.h"

namespace cse498 {

constexpr const int TIMEOUT = 4000;

MainWindow::MainWindow(SimWorldBase<DiseaseData> &world,
                       const std::vector<QString> &imagePaths, int tileSize,
                       const QString &agentImagePath, QWidget *parent, int mode)
    : QMainWindow(parent),
      mWorld(world),
      mImagePaths(imagePaths),
      mTileSize(tileSize),
      mAgentImagePath(agentImagePath),
      mMode(mode) {
  setWindowTitle("Group 21 Demo- Virus");
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

    //clears agents if restarting
    iw.ClearAgents();


    auto makeData = [](size_t x, size_t y) {
      DiseaseData d;
      d.position = WorldPosition{x, y};
      return d;
    };

    iw.AddAgent<StepPacingAgent>(makeData(3, 2));
    iw.AddAgent<StepPacingAgent>(makeData(5, 4));
    iw.AddAgent<StepPacingAgent>(makeData(7, 3));
    iw.AddAgent<StepPacingAgent>(makeData(4, 6));
    iw.AddAgent<StepPacingAgent>(makeData(8, 7));
    iw.AddAgent<StepPacingAgent>(makeData(12, 3));
    iw.AddAgent<StepPacingAgent>(makeData(15, 5));
    iw.AddAgent<StepPacingAgent>(makeData(14, 2));
    iw.AddAgent<StepPacingAgent>(makeData(16, 7));
    iw.AddAgent<StepPacingAgent>(makeData(13, 7));
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

  mExitAction =
      new QAction(QIcon::fromTheme("application-exit"), ("E&xit"), this);
  mExitAction->setShortcut(QKeySequence::Quit);
  mExitAction->setStatusTip("Exit the application");
  connect(mExitAction, &QAction::triggered, this, &MainWindow::onFileExit);

  mFileMenu->addAction(mNewFileAction);
  mFileMenu->addAction(mOpenFileAction);
  mFileMenu->addAction(mSaveFileAction);
  mFileMenu->addSeparator();
  mFileMenu->addAction(mExitAction);

  // Main Menu dropdown
  mMainMenu = menuBar()->addMenu("&Main Menu");

  mSwitchToTrafficAction = new QAction("Switch to Traffic Simulation", this);
  mSwitchToTrafficAction->setStatusTip("Switch to the traffic simulation");
  connect(mSwitchToTrafficAction, &QAction::triggered, this,
          &MainWindow::onSwitchToTrafficSimulation);

  mSwitchToInfectionAction = new QAction("Switch to Infection Simulation", this);
  mSwitchToInfectionAction->setStatusTip("Switch to the infection simulation");
  connect(mSwitchToInfectionAction, &QAction::triggered, this,
          &MainWindow::onSwitchToInfectionSimulation);

  mReturnHomeAction =
      new QAction(QIcon::fromTheme("go-home"), "Return to Home Screen", this);
  mReturnHomeAction->setStatusTip("Return to the home screen");
  connect(mReturnHomeAction, &QAction::triggered, this,
          &MainWindow::onReturnToHomeScreen);

  mAboutAction = new QAction(QIcon::fromTheme("help-about"), ("&About"), this);
  mAboutAction->setStatusTip("Show information about this application");
  connect(mAboutAction, &QAction::triggered, this, &MainWindow::onHelpAbout);

  mMainMenu->addAction(mSwitchToTrafficAction);
  mMainMenu->addAction(mSwitchToInfectionAction);
  mMainMenu->addSeparator();
  mMainMenu->addAction(mReturnHomeAction);
  mMainMenu->addSeparator();
  mMainMenu->addAction(mAboutAction);

  if (mMode == 1) {
    mSwitchToTrafficAction->setEnabled(false);
  }

  if (mMode == 2) {
    mSwitchToInfectionAction->setEnabled(false);
  }

  // Help menu
  mHelpMenu = menuBar()->addMenu("&Help");

  mHelpSimulationsAction = new QAction("Simulation Information", this);
  mHelpSimulationsAction->setStatusTip("Show information about the simulations");
  connect(mHelpSimulationsAction, &QAction::triggered, this,
          &MainWindow::onHelpSimulations);

  mHelpMenu->addAction(mHelpSimulationsAction);

  mToolBar = addToolBar("Simulation");
  mToolBar->setMovable(false);

  mReplayToggleAction = new QAction("Pause", this);
  mReplayToggleAction->setShortcut(QKeySequence(Qt::Key_Space));
  mReplayToggleAction->setStatusTip("Pause or resume the simulation");
  connect(mReplayToggleAction, &QAction::triggered, this,
          &MainWindow::onReplayToggle);
  mToolBar->addAction(mReplayToggleAction);

  mReplayRestartAction = new QAction("Restart", this);
  mReplayRestartAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
  mReplayRestartAction->setStatusTip("Reset and restart the simulation");
  connect(mReplayRestartAction, &QAction::triggered, this,
          &MainWindow::onReplayRestart);
  mToolBar->addAction(mReplayRestartAction);
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

void MainWindow::onHelpSimulations() {
  QString message;

  if (mMode == 1) {
    message =
        "<b>Traffic Simulation</b><br><br>"
        "The Traffic simulation displays a road-based world where cars or agents "
        "move through the grid. The graph options can show traffic-related data, "
        "such as waiting cars, driving cars, active cars, distance driven, and "
        "time to arrive.<br><br>"
        "<b>How to use it:</b><br>"
        "- Use the pause/play button to stop or resume the simulation.<br>"
        "- Use restart to reset the simulation.<br>"
        "- Use the Graphs menu to switch between traffic graphs.<br>"
        "- Use Main Menu to switch to the Infection simulation or return home.";
  } else {
    message =
        "<b>Infection Simulation</b><br><br>"
        "The Infection simulation displays agents moving through a world where "
        "infection can spread between nearby agents. The graph options can show "
        "infection-related data, such as infected, susceptible, and cured counts."
        "<br><br>"
        "<b>How to use it:</b><br>"
        "- Use the pause/play button to stop or resume the simulation.<br>"
        "- Use restart to reset the simulation.<br>"
        "- Use the Graphs menu to switch between infection graphs.<br>"
        "- Use Main Menu to switch to the Traffic simulation or return home.";
  }

  QMessageBox::information(this, "Simulation Information", message);
}

void MainWindow::setOwnedWorld(std::unique_ptr<WorldBase> world) {
  mOwnedWorld = std::move(world);
}

void MainWindow::onReturnToHomeScreen() {
  if (mTimer) {
    mTimer->stop();
  }

  auto *startScreen = new StartScreen(mImagePaths, mTileSize, mAgentImagePath);
  startScreen->setAttribute(Qt::WA_DeleteOnClose);
  startScreen->show();

  close();
}

void MainWindow::onSwitchToTrafficSimulation() {
  if (mMode == 1) {
    statusBar()->showMessage("Already in Traffic Simulation", TIMEOUT);
    return;
  }

  if (mTimer) {
    mTimer->stop();
  }

  emit requestSimulationSwitch("traffic");

  close();
}

void MainWindow::onSwitchToInfectionSimulation() {
  if (mMode == 2) {
    statusBar()->showMessage("Already in Infection Simulation", TIMEOUT);
    return;
  }

  if (mTimer) {
    mTimer->stop();
  }

  emit requestSimulationSwitch("infection");

  close();
}

void MainWindow::logCommand(const QString &message) {
  mCommandLog->append(message);
}

void MainWindow::onReplayToggle() {
  if (mIsRunning) {
    mTimer->stop();
    mReplayToggleAction->setText("Play");
    statusBar()->showMessage("Simulation paused", TIMEOUT);
    logCommand("[System] Simulation paused.");
  } else {
    mTimer->start(mTickInterval);
    mReplayToggleAction->setText("Pause");
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
  mReplayToggleAction->setText("Pause");
  mTimer->start(mTickInterval);

  statusBar()->showMessage("Simulation restarted", TIMEOUT);
  logCommand("[System] Simulation restarted.");
}

}  // namespace cse498