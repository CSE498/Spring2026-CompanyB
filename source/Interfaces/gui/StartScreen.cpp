#include "StartScreen.hpp"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QVBoxLayout>

namespace cse498 {

/// Constructor
StartScreen::StartScreen(WorldBase& world,
                         const std::vector<QString>& imagePaths, int tileSize,
                         QWidget* parent)
    : QWidget(parent),
      mWorld(world),
      mImagePaths(imagePaths),
      mTileSize(tileSize) {
  setWindowTitle("Main Menu");
  setMinimumSize(640, 480);
  resize(800, 480);

  // Dark Background
  auto* outerLayout = new QVBoxLayout(this);
  outerLayout->setContentsMargins(10, 10, 10, 10);

  // "Main Menu" label
  auto* menuLabel = new QLabel("Main Menu", this);
  menuLabel->setStyleSheet("color: white; font-size: 16px;");
  outerLayout->addWidget(menuLabel);

  // Green card outline
  auto* card = new QWidget(this);
  card->setObjectName("card");
  card->setStyleSheet(R"(
        QWidget#card {
            background-color: #18453B;
            border: 6px solid #FFFFFF;
            border-radius: 4px;
        }
    )");
  outerLayout->addWidget(card);

  // Card layout
  auto* cardLayout = new QVBoxLayout(card);
  cardLayout->setContentsMargins(40, 40, 40, 40);

  // "Simulation Tool" title
  cardLayout->addStretch();
  auto* title = new QLabel("Simulation Tool", card);
  title->setAlignment(Qt::AlignCenter);
  title->setStyleSheet(
      "color: white; font-size: 30px; font-weight: 300; letter-spacing: 2px;");
  cardLayout->addWidget(title);

  cardLayout->addSpacing(20);

  // Traffic / Virus buttons
  mTrafficBtn = new QPushButton("Traffic", card);
  mVirusBtn = new QPushButton("Virus", card);

  const QString btnStyle = R"(
        QPushButton {
            background-color: #1a1a1a;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 10px 32px;
            font-size: 14px;
            min-width: 110px;
        }
        QPushButton:hover  { background-color: #333333; }
        QPushButton:pressed{ background-color: #000000; }
    )";
  mTrafficBtn->setStyleSheet(btnStyle);
  mVirusBtn->setStyleSheet(btnStyle);

  // buttons for traffic + virus
  auto* btnRow = new QHBoxLayout();
  btnRow->setAlignment(Qt::AlignCenter);
  btnRow->setSpacing(16);
  btnRow->addWidget(mTrafficBtn);
  btnRow->addWidget(mVirusBtn);
  cardLayout->addLayout(btnRow);

  cardLayout->addStretch();

  // connects clicked
  connect(mTrafficBtn, &QPushButton::clicked, this,
          &StartScreen::onTrafficClicked);
  connect(mVirusBtn, &QPushButton::clicked, this, &StartScreen::onVirusClicked);

  // Dark window background
  setStyleSheet("QWidget { background-color: #1e1e1e; }");
}

void StartScreen::onTrafficClicked() { launchMainWindow("Traffic"); }
void StartScreen::onVirusClicked() { launchMainWindow("Infection"); }

void StartScreen::launchMainWindow(const QString& mode) {
  auto* win = new MainWindow(mWorld, mImagePaths, mTileSize);
  win->setWindowTitle(QString("Group 21 Demo – %1").arg(mode));
  win->setAttribute(Qt::WA_DeleteOnClose);
  win->show();
  close();  // close the start screen
}

}  // namespace cse498