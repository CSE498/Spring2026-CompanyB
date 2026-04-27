#include "StartScreen.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QVBoxLayout>

namespace cse498 {

StartScreen::StartScreen(const std::vector<QString>& imagePaths, int tileSize,
                         const QString& agentImagePath, QWidget* parent)
    : QWidget(parent),
      mImagePaths(imagePaths),
      mTileSize(tileSize),
      mAgentImagePath(agentImagePath) {
  setWindowTitle("Main Menu");
  setMinimumSize(640, 480);
  resize(800, 480);

  auto* outerLayout = new QVBoxLayout(this);
  outerLayout->setContentsMargins(10, 10, 10, 10);

  auto* menuLabel = new QLabel("Main Menu", this);
  menuLabel->setStyleSheet("color: white; font-size: 16px;");
  outerLayout->addWidget(menuLabel);

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

  auto* cardLayout = new QVBoxLayout(card);
  cardLayout->setContentsMargins(40, 40, 40, 40);

  cardLayout->addStretch();
  auto* title = new QLabel("Simulation Tool", card);
  title->setAlignment(Qt::AlignCenter);
  title->setStyleSheet(
      "color: white; font-size: 30px; font-weight: 300; letter-spacing: 2px;");
  cardLayout->addWidget(title);

  cardLayout->addSpacing(20);

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

  auto* btnRow = new QHBoxLayout();
  btnRow->setAlignment(Qt::AlignCenter);
  btnRow->setSpacing(16);
  btnRow->addWidget(mTrafficBtn);
  btnRow->addWidget(mVirusBtn);
  cardLayout->addLayout(btnRow);

  cardLayout->addStretch();

  connect(mTrafficBtn, &QPushButton::clicked, this,
          &StartScreen::onTrafficClicked);
  connect(mVirusBtn, &QPushButton::clicked, this, &StartScreen::onVirusClicked);

  setStyleSheet("QWidget { background-color: #1e1e1e; }");
}

void StartScreen::onTrafficClicked() { launchMainWindow(1); }
void StartScreen::onVirusClicked() { launchMainWindow(2); }

void StartScreen::launchMainWindow(int mode) {
  if (mode == 1) {
    mTrafficWorld =
        std::make_unique<StepTrafficWorld<SwarmingAgent<TrafficData>>>(
            "assets/grids/DemoWorld.grid"  // adjust path if needed
        );

    auto* win = new TrafficMainWindow(*mTrafficWorld, mImagePaths, mTileSize ,
                                      mAgentImagePath);
    win->mOwnedWorld = std::move(mTrafficWorld);
    win->setWindowTitle("Group 21 Demo - Traffic");
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->show();

  } else {
    mDiseaseWorld = std::make_unique<InfectiousWorld>(20, 15);
    mDiseaseWorld->SetTransmissionRate(0.4);
    mDiseaseWorld->SetInfectionRadius(1.5);
    mDiseaseWorld->SetInfectionDuration(8);
    mDiseaseWorld->SetImmunityDuration(15);

    auto* win = new MainWindow(*mDiseaseWorld, mImagePaths, mTileSize ,
                               mAgentImagePath, nullptr, 2);
    win->mOwnedWorld = std::move(mDiseaseWorld);
    win->setWindowTitle("Group 21 Demo - Virus");
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->show();
  }

  close();
}
}  // namespace cse498