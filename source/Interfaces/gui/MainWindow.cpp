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

namespace cse498 {

constexpr const int TIMEOUT = 4000;

MainWindow::MainWindow(WorldBase &world, const std::vector<QString> &imagePaths,
                       int tileSize, const QString &agentImagePath, QWidget *parent, int mode)
    : QMainWindow(parent),
      mWorld(world),
      mImagePaths(imagePaths),
      mTileSize(tileSize),
      mAgentImagePath(agentImagePath),
      mMode(mode)
{
    setWindowTitle("Group 21 Demo");
    setMinimumSize(640, 480);
    resize(1280, 720);

    setMenuBar();
    setMainWidget();
    setupAgents();
    setImageGrid();
    setStatusBar();
    startSimulation();
}

void MainWindow::setupAgents() {
    if (mMode == 1) {
        mWorld.AddAgent<PacingAgent>("Agent-1").SetLocation(WorldPosition{3, 2});
    } else {
        auto& iw = static_cast<InfectiousWorld&>(mWorld);
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

    mNewFileAction = new QAction(QIcon::fromTheme("document-new"), ("&New File..."), this);
    mNewFileAction->setShortcut(QKeySequence::New);
    mNewFileAction->setStatusTip("Create a new file");
    connect(mNewFileAction, &QAction::triggered, this, &MainWindow::onFileNew);

    mOpenFileAction = new QAction(QIcon::fromTheme("document-open"), ("&Open File..."), this);
    mOpenFileAction->setShortcut(QKeySequence::Open);
    mOpenFileAction->setStatusTip("Open an existing file");
    connect(mOpenFileAction, &QAction::triggered, this, &MainWindow::onFileOpen);

    mSaveFileAction = new QAction(QIcon::fromTheme("document-save"), ("&Save"), this);
    mSaveFileAction->setShortcut(QKeySequence::Save);
    mSaveFileAction->setStatusTip("Save the current file");
    connect(mSaveFileAction, &QAction::triggered, this, &MainWindow::onFileSave);

    mExitAction = new QAction(QIcon::fromTheme("application-exit"), ("E&xit"), this);
    mExitAction->setShortcut(QKeySequence::Quit);
    mExitAction->setStatusTip("Exit the application");
    connect(mExitAction, &QAction::triggered, this, &MainWindow::onFileExit);

    mFileMenu->addAction(mNewFileAction);
    mFileMenu->addAction(mOpenFileAction);
    mFileMenu->addAction(mSaveFileAction);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mExitAction);

    mHelpMenu = menuBar()->addMenu("&Help");
    mAboutAction = new QAction(QIcon::fromTheme("help-about"), ("&About"), this);
    mAboutAction->setStatusTip("Show information about this application");
    connect(mAboutAction, &QAction::triggered, this, &MainWindow::onHelpAbout);
    mHelpMenu->addAction(mAboutAction);
}

void MainWindow::setMainWidget() {
    mGraphicsScene = new QGraphicsScene(this);
    mGraphicsView = new QGraphicsView(mGraphicsScene, this);
    mGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    mSidePanel = new QWidget(this);
    mSidePanel->setMinimumWidth(200);

    mMainGraph = new MainGraph(mSidePanel);
    mSidePanelLayout = new QVBoxLayout(mSidePanel);
    mSidePanelLayout->addWidget(mMainGraph);

    mCommandLog = new QTextEdit(this);
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
    mSidePanelLayout->addWidget(mCommandLog, 1);
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
        this, "Open File", QString(), ("All Files (*.*)"));
    if (path.isEmpty()) return;

    std::ifstream input(path.toStdString());
    if (!input.is_open()) {
        QMessageBox::warning(this, "Open Failed", "Could not open file.");
        return;
    }
    mWorld.GetGrid().Load(input);

    std::string line, log;
    bool inLog = false;
    while (std::getline(input, line)) {
        if (line == "--- AGENT LOG ---") { inLog = true; continue; }
        if (inLog) log += line + "\n";
    }

    mGraphicsScene->clear();
    setImageGrid();

    if (!log.empty())
        mCommandLog->setPlainText(QString::fromStdString(log));

    statusBar()->showMessage(QString("Opened: %1").arg(path), TIMEOUT);
}

void MainWindow::onFileSave() {
    const QString path = QFileDialog::getSaveFileName(
        this, "Save File", QString(), "Text Files (*.txt);;All Files (*.*)");
    if (path.isEmpty()) return;

    std::ofstream output(path.toStdString());
    if (!output.is_open()) {
        QMessageBox::warning(this, "Save Failed", "Could not save file.");
        return;
    }

    mWorld.GetGrid().Print(output);
    output << mCommandLog->toPlainText().toStdString();
    statusBar()->showMessage(QString("File Saved: %1").arg(path), TIMEOUT);
}

void MainWindow::onFileExit() { close(); }

void MainWindow::onHelpAbout() {
    QMessageBox::about(this, "About", "<b>Group 21 Demo</b>");
}

void MainWindow::logCommand(const QString& message) {
    mCommandLog->append(message);
}

} // namespace cse498