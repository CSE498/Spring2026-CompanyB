#include "InfectiousWindow.hpp"

#include <QApplication>
#include <QBrush>
#include <QFileDialog>
#include <QMessageBox>
#include <QPen>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>
#include <fstream>
#include <string>
#include <vector>

#include "StartScreen.h"

// based on TrafficMainWindow, adapted for infectious simulation
namespace cse498 {

constexpr int INFECTIOUS_TIMEOUT = 4000;

InfectiousWindow::InfectiousWindow(InfectiousWorld& world,
                                   const std::vector<QString>& imagePaths,
                                   int tileSize, const QString& agentImagePath,
                                   QWidget* parent)
    : QMainWindow(parent),
      mWorld(world),
      mImagePaths(imagePaths),
      mTileSize(tileSize),
      mAgentImagePath(agentImagePath) {
  setWindowTitle("Group 21 Demo - Infectious Disease");
  setMinimumSize(640, 480);
  resize(1280, 720);

  setupMenuBar();
  setupMainWidget();
  setupStatusBar();
  startSimulation();
}

void InfectiousWindow::setupMenuBar() {
  mFileMenu = menuBar()->addMenu("&File");

  mNewFileAction =
      new QAction(QIcon::fromTheme("document-new"), "&New File...", this);
  mNewFileAction->setShortcut(QKeySequence::New);
  mNewFileAction->setStatusTip("Create a new file");
  connect(mNewFileAction, &QAction::triggered, this,
          &InfectiousWindow::onFileNew);

  mOpenFileAction =
      new QAction(QIcon::fromTheme("document-open"), "&Open File...", this);
  mOpenFileAction->setShortcut(QKeySequence::Open);
  mOpenFileAction->setStatusTip("Open an existing file");
  connect(mOpenFileAction, &QAction::triggered, this,
          &InfectiousWindow::onFileOpen);

  mSaveFileAction =
      new QAction(QIcon::fromTheme("document-save"), "&Save", this);
  mSaveFileAction->setShortcut(QKeySequence::Save);
  mSaveFileAction->setStatusTip("Save the simulation log");
  connect(mSaveFileAction, &QAction::triggered, this,
          &InfectiousWindow::onFileSave);

  mBackToMenuAction =
      new QAction(QIcon::fromTheme("go-home"), "&Back to Main Menu", this);
  mBackToMenuAction->setStatusTip("Return to the main menu");
  connect(mBackToMenuAction, &QAction::triggered, this,
          &InfectiousWindow::onBackToMainMenu);

  mExitAction =
      new QAction(QIcon::fromTheme("application-exit"), "E&xit", this);
  mExitAction->setShortcut(QKeySequence::Quit);
  mExitAction->setStatusTip("Exit the application");
  connect(mExitAction, &QAction::triggered, this,
          &InfectiousWindow::onFileExit);

  mFileMenu->addAction(mNewFileAction);
  mFileMenu->addAction(mOpenFileAction);
  mFileMenu->addAction(mSaveFileAction);
  mFileMenu->addSeparator();
  mFileMenu->addAction(mBackToMenuAction);
  mFileMenu->addSeparator();
  mFileMenu->addAction(mExitAction);

  mHelpMenu = menuBar()->addMenu("&Help");
  mAboutAction = new QAction(QIcon::fromTheme("help-about"), "&About", this);
  mAboutAction->setStatusTip("Show information about this application");
  connect(mAboutAction, &QAction::triggered, this,
          &InfectiousWindow::onHelpAbout);
  mHelpMenu->addAction(mAboutAction);

  mToolBar = addToolBar("Simulation");
  mToolBar->setMovable(false);

  mReplayToggleAction = new QAction("Pause", this);
  mReplayToggleAction->setShortcut(QKeySequence(Qt::Key_Space));
  mReplayToggleAction->setStatusTip("Pause or resume the simulation");
  connect(mReplayToggleAction, &QAction::triggered, this,
          &InfectiousWindow::onReplayToggle);
  mToolBar->addAction(mReplayToggleAction);

  mReplayRestartAction = new QAction("Restart", this);
  mReplayRestartAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
  mReplayRestartAction->setStatusTip("Reset and restart the simulation");
  connect(mReplayRestartAction, &QAction::triggered, this,
          &InfectiousWindow::onReplayRestart);
  mToolBar->addAction(mReplayRestartAction);
}

void InfectiousWindow::setupMainWidget() {
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
      "  background-color: #2d3a2d;"
      "  color: #e0e0e0;"
      "  border: 2px solid #1a2d1a;"
      "  border-radius: 8px;"
      "  padding: 8px;"
      "  font-family: monospace;"
      "  font-size: 12px;"
      "}");
  mCommandLog->setPlaceholderText("Simulation events will appear here...");
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

void InfectiousWindow::setupStatusBar() {
  statusBar()->showMessage("Infectious Simulation", INFECTIOUS_TIMEOUT);
}

void InfectiousWindow::startSimulation() {
  mTimer = new QTimer(this);
  connect(mTimer, &QTimer::timeout, this, &InfectiousWindow::onTick);
  mTimer->start(mTickInterval);
  redraw();
}

void InfectiousWindow::onTick() {
  mWorld.UpdateWorld();  // InfectiousWorld::UpdateWorld runs agents internally
  ++mTickCount;
  redraw();

  if (mTickCount % 10 == 0) {
    logCommand(
        QString("[Tick %1]  Susceptible: %2  |  Infected: %3  |  Recovered: %4")
            .arg(mTickCount)
            .arg(mWorld.GetSusceptibleCount())
            .arg(mWorld.GetInfectedCount())
            .arg(mWorld.GetRecoveredCount()));
  }
}

// rendering
void InfectiousWindow::redraw() {
  mGraphicsScene->clear();

  const WorldGrid& grid = mWorld.GetGrid();
  const int T = mTileSize;
  const size_t W = grid.GetWidth();
  const size_t H = grid.GetHeight();

  // draw grid tiles
  for (size_t y = 0; y < H; ++y) {
    for (size_t x = 0; x < W; ++x) {
      WorldPosition pos{x, y};
      QColor colour;

      if (mWorld.IsInQuarantine(pos)) {
        colour = QColor(180, 140, 30);  // yellow quarantine zone
      } else {
        char sym = grid.GetSymbol(pos);
        if (sym == '#') {
          colour = QColor(50, 50, 50);  // wall
        } else {
          colour = QColor(90, 110, 90);  // floor
        }
      }

      mGraphicsScene->addRect(static_cast<int>(x) * T, static_cast<int>(y) * T,
                              T, T, QPen(Qt::NoPen), QBrush(colour));
    }
  }

  // draw agents
  for (size_t i = 0; i < mWorld.GetNumAgents(); ++i) {
    DiseaseData state = mWorld.GetAgentState(i);
    const int ax = static_cast<int>(state.position.CellX()) * T;
    const int ay = static_cast<int>(state.position.CellY()) * T;

    QColor agentColour;
    switch (state.health) {
      case HealthState::INFECTED:
        agentColour = QColor(220, 50, 50);  // red
        break;
      case HealthState::RECOVERED:
        agentColour = QColor(50, 100, 220);  // blue
        break;
      case HealthState::SUSCEPTIBLE:
      default:
        agentColour = QColor(60, 200, 60);  // green
        break;
    }

    mGraphicsScene->addEllipse(ax + 2, ay + 2, T - 4, T - 4, QPen(Qt::NoPen),
                               QBrush(agentColour));
  }
}

void InfectiousWindow::logCommand(const QString& message) {
  mCommandLog->append(message);
}

// menu & toolbar
void InfectiousWindow::onFileNew() {
  statusBar()->showMessage("New file created", INFECTIOUS_TIMEOUT);
}

void InfectiousWindow::onFileOpen() {
  const QString path =
      QFileDialog::getOpenFileName(this, "Open File", QString(),
                                   "Text Files (*.txt *.sim);;All Files (*.*)");
  if (path.isEmpty()) return;
  statusBar()->showMessage(QString("Opened: %1").arg(path), INFECTIOUS_TIMEOUT);
}

void InfectiousWindow::onFileSave() {
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
  output << "--- SIM DATA ---\n";
  output << "tick=" << mTickCount << "\n";
  output << "susceptible=" << mWorld.GetSusceptibleCount() << "\n";
  output << "infected=" << mWorld.GetInfectedCount() << "\n";
  output << "recovered=" << mWorld.GetRecoveredCount() << "\n";

  statusBar()->showMessage(QString("Saved: %1").arg(path), INFECTIOUS_TIMEOUT);
}

void InfectiousWindow::onFileExit() { close(); }

void InfectiousWindow::onHelpAbout() {
  QMessageBox::about(this, "About",
                     "<b>Group 21 Demo</b><br>Infectious Simulation");
}

void InfectiousWindow::onBackToMainMenu() {
  if (mTimer) mTimer->stop();
  auto* startScreen = new StartScreen(mImagePaths, mTileSize, mAgentImagePath);
  startScreen->setAttribute(Qt::WA_DeleteOnClose);
  startScreen->show();
  close();
}

void InfectiousWindow::onReplayToggle() {
  if (mIsRunning) {
    mTimer->stop();
    mReplayToggleAction->setText("Play");
    statusBar()->showMessage("Simulation paused", INFECTIOUS_TIMEOUT);
    logCommand("[System] Simulation paused.");
  } else {
    mTimer->start(mTickInterval);
    mReplayToggleAction->setText("Pause");
    statusBar()->showMessage("Simulation resumed", INFECTIOUS_TIMEOUT);
    logCommand("[System] Simulation resumed.");
  }
  mIsRunning = !mIsRunning;
}

void InfectiousWindow::onReplayRestart() {
  mTimer->stop();
  mTickCount = 0;
  mCommandLog->clear();
  mIsRunning = true;
  mReplayToggleAction->setText("Pause");
  mTimer->start(mTickInterval);
  statusBar()->showMessage("Simulation restarted", INFECTIOUS_TIMEOUT);
  logCommand("[System] Simulation restarted.");
}

void InfectiousWindow::onShowSimulationHelp() {
  QMessageBox::information(
      this, "Infection Simulation Help",
      "Current Simulation: Virus / Infection Simulation\n\n"
      "This simulation shows agents moving through a grid-based world while "
      "infection spreads between agents. The display updates every tick as "
      "agents move and the world state changes.\n\n"
      "Use Pause to stop the simulation, Restart to reset it, and Main Menu "
      "to switch simulations or return to the start screen.");
}

void InfectiousWindow::onSwitchToInfectiousSimulation() {
  statusBar()->showMessage("Already in Infectious Simulation",
                           INFECTIOUS_TIMEOUT);
}

void InfectiousWindow::onSwitchToTrafficSimulation() {
  if (mTimer) {
    mTimer->stop();
  }

  // TODO: switch to traffic sim
}

}  // namespace cse498