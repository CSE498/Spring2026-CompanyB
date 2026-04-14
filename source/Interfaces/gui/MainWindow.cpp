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

namespace cse498
{

constexpr const int TIMEOUT = 4000;  // time until status bar messages timeout; 4 seconds

    MainWindow::MainWindow(WorldBase &world, const std::vector<QString> &imagePaths,
                           int tileSize, const QString &agentImagePath, QWidget *parent)
        : QMainWindow(parent),
          mWorld(world),
          mImagePaths(imagePaths),
          mAgentImagePath(agentImagePath),
          mTileSize(tileSize)
    {

        setWindowTitle("Group 21 Demo");
        setMinimumSize(640, 480); // min size the window can be
        resize(1280, 720);        // initial size of the window

        setMenuBar();
        setMainWidget();
        setImageGrid();
        setStatusBar();

        startSimulation();
    }

    void MainWindow::startSimulation()
    {
        mTimer = new QTimer(this);
        connect(mTimer, &QTimer::timeout, this, &MainWindow::onTick);
        mTimer->start(mTickInterval);
    }

    void MainWindow::onTick()
    {
        mWorld.RunAgents();
        mWorld.UpdateWorld();

        mImageGrid->RenderGrid();
        mImageGrid->RenderAgents();
    
    logCommand("TEST Tick 1: [Agent 0] Move forward TEST");
    logCommand("TEST Tick 2: [Agent 1] Stop TEST");
}

    void MainWindow::setMenuBar()
    {
        // https://doc.qt.io/qt-6/qkeysequence.html

        // Add file menu
        mFileMenu = menuBar()->addMenu("&File");

        // File > New File

        // create new action
        mNewFileAction =
            new QAction(QIcon::fromTheme("document-new"), ("&New File..."), this);
        // bind keyboard shortcut
        mNewFileAction->setShortcut(QKeySequence::New);
        // set status text
        mNewFileAction->setStatusTip("Create a new file");
        // connect this action to onFileNew()
        connect(mNewFileAction, &QAction::triggered, this, &MainWindow::onFileNew);

        // File > Open File
        mOpenFileAction =
            new QAction(QIcon::fromTheme("document-open"), ("&Open File..."), this);
        mOpenFileAction->setShortcut(QKeySequence::Open);
        mOpenFileAction->setStatusTip("Open an existing file");
        connect(mOpenFileAction, &QAction::triggered, this,
                &MainWindow::onFileOpen);

        // File > Save
        mSaveFileAction =
            new QAction(QIcon::fromTheme("document-save"), ("&Save"), this);
        mSaveFileAction->setShortcut(QKeySequence::Save);
        mSaveFileAction->setStatusTip("Save the current file");
        connect(mSaveFileAction, &QAction::triggered, this,
                &MainWindow::onFileSave);

        // File > Exit
        mExitAction =
            new QAction(QIcon::fromTheme("application-exit"), ("E&xit"), this);
        mExitAction->setShortcut(QKeySequence::Quit);
        mExitAction->setStatusTip("Exit the application");
        connect(mExitAction, &QAction::triggered, this, &MainWindow::onFileExit);

        // add actions to the File menu
        mFileMenu->addAction(mNewFileAction);
        mFileMenu->addAction(mOpenFileAction);
        mFileMenu->addAction(mSaveFileAction);
        mFileMenu->addSeparator();
        mFileMenu->addAction(mExitAction);

        // Help menu
        mHelpMenu = menuBar()->addMenu("&Help");

        mAboutAction =
            new QAction(QIcon::fromTheme("help-about"), ("&About"), this);
        mAboutAction->setStatusTip("Show information about this application");
        connect(mAboutAction, &QAction::triggered, this, &MainWindow::onHelpAbout);

        // add action to the Help Menu
        mHelpMenu->addAction(mAboutAction);
    }

    void MainWindow::setMainWidget()
    {
        mGraphicsScene = new QGraphicsScene(this);

        mGraphicsView = new QGraphicsView(mGraphicsScene, this);
        mGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    // blank panel (placeholder for graph and log)
    mSidePanel = new QWidget(this);
    mSidePanel->setMinimumWidth(200);

    // graph display on panel
    mMainGraph = new MainGraph(mSidePanel);

    mSidePanelLayout = new QVBoxLayout(mSidePanel);
    mSidePanelLayout->addWidget(mMainGraph);

    // Command log
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

        // horizontal splitter w blank panel on the left, image grid on the right
        auto *splitter = new QSplitter(Qt::Horizontal, this);
        splitter->addWidget(mSidePanel);
        splitter->addWidget(mGraphicsView);

        // blank panel is pos 0, imagegrid is 1 and gets 3x space
        // https://doc.qt.io/qt-6/qsplitter.html
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 3);

        setCentralWidget(splitter);
    }

    void MainWindow::setImageGrid()
    {
        mImageGrid =
            std::make_unique<ImageGrid>(mWorld, *mGraphicsScene, mTileSize);

        mImageGrid->MapImages(mImagePaths);
        mImageGrid->LoadAgentImage(mAgentImagePath);
        mImageGrid->RenderGrid();
        mImageGrid->RenderAgents();
        mImageGrid->SetSceneAndView(*mGraphicsView);
    }

    void MainWindow::setStatusBar()
    {
        // ignore the silly placeholder
        statusBar()->showMessage("This is a status bar!", TIMEOUT);
    }

    // File management

    void MainWindow::onFileNew()
    {
        statusBar()->showMessage("New file created", TIMEOUT);

        // TODO: implement new file logic
    }

    void MainWindow::onFileOpen()
    {
        const QString path = QFileDialog::getOpenFileName(
            this, "Open File", QString(), ("All Files (*.*)"));

    if (path.isEmpty()) {
        return;
    }

    std::ifstream input(path.toStdString());
    if (!input.is_open()) {
        QMessageBox::warning(this, "Open Failed", "Could not open file.");
        return;
    }
    mWorld.GetGrid().Load(input);

    mGraphicsScene->clear();
    setImageGrid();

    statusBar()->showMessage(QString("Opened: %1").arg(path), TIMEOUT);
}

void MainWindow::onFileSave() {
    // show message for 2 sec
    const QString path = QFileDialog::getSaveFileName(
        this, "Save File", QString(), "All Files (*.*)");

    if (path.isEmpty()) {
        return;
    }
    std::ofstream output(path.toStdString());
    if (!output.is_open()) {
        QMessageBox::warning(this, "Save Failed", "Could not save file.");
        return;
    }

    mWorld.GetGrid().Print(output);

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