#include "StartScreen.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QVBoxLayout>
#include <format>
#include <string>
#include <string_view>

namespace cse498 {

// copies from Group13_main.cpp for infectious simulation

constexpr std::string_view BASE_SCRIPT = R"V0G0N(
world infection;

fn opp_dir(prev_dir : direction) : direction {
  if (prev_dir == left) return right;
  else if (prev_dir == right) return left;
  else if (prev_dir == up) return down;
  else return up;
};

)V0G0N";

constexpr std::string_view AGENT_FORMAT_STR = R"V0G0N(
let agent_{0} : student {{
  init: {{
    __spawn__ = make_point({1}, {2});
    let prev_dir_lr : direction = left;
    let prev_dir_ud : direction = up;
  }};
  turn: {{
    prev_dir_lr = opp_dir(prev_dir_lr);
    prev_dir_ud = opp_dir(prev_dir_ud);

    if (__recovered__) move(prev_dir_ud);
    else if (!__infected__) move(prev_dir_lr);
  }};
}};

)V0G0N";

static std::string MakeScript(
    std::initializer_list<std::pair<size_t, size_t>> positions) {
  std::string result{BASE_SCRIPT};
  size_t agent_count = 0;
  for (auto [x, y] : positions) {
    result += std::format(AGENT_FORMAT_STR, agent_count++, x, y);
  }
  return result;
}

static void DrawBuilding(
    WorldGrid &grid, size_t wall_id, size_t floor_id, size_t x1, size_t y1,
    size_t x2, size_t y2,
    std::initializer_list<std::pair<size_t, size_t>> doors) {
  for (size_t x = x1; x <= x2; ++x) {
    grid[x, y1] = wall_id;
    grid[x, y2] = wall_id;
  }
  for (size_t y = y1 + 1; y < y2; ++y) {
    grid[x1, y] = wall_id;
    grid[x2, y] = wall_id;
  }
  for (auto [dx, dy] : doors) grid[dx, dy] = floor_id;
}

StartScreen::StartScreen(const std::vector<QString> &imagePaths, int tileSize,
                         const QString &agentImagePath, QWidget *parent)
    : QWidget(parent),
      mImagePaths(imagePaths),
      mTileSize(tileSize),
      mAgentImagePath(agentImagePath) {
  setWindowTitle("Main Menu");
  setMinimumSize(640, 480);
  resize(800, 480);

  auto *outerLayout = new QVBoxLayout(this);
  outerLayout->setContentsMargins(10, 10, 10, 10);

  auto *menuLabel = new QLabel("Main Menu", this);
  menuLabel->setStyleSheet("color: white; font-size: 16px;");
  outerLayout->addWidget(menuLabel);

  auto *card = new QWidget(this);
  card->setObjectName("card");
  card->setStyleSheet(R"(
        QWidget#card {
            background-color: #18453B;
            border: 6px solid #FFFFFF;
            border-radius: 4px;
        }
    )");
  outerLayout->addWidget(card);

  auto *cardLayout = new QVBoxLayout(card);
  cardLayout->setContentsMargins(40, 40, 40, 40);

  cardLayout->addStretch();
  auto *title = new QLabel("Simulation Tool", card);
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

  auto *btnRow = new QHBoxLayout();
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
  // mode 1 == traffic sim; else == infectious sim
  if (mode == 1) {
    mTrafficWorld =
        std::make_unique<StepTrafficWorld<SwarmingAgent<TrafficData>>>(
            "source/DemoWorld.grid");

    auto *win = new TrafficMainWindow(*mTrafficWorld, mImagePaths, mTileSize,
                                      mAgentImagePath);
    win->mOwnedWorld = std::move(mTrafficWorld);
    win->setWindowTitle("Group 21 Demo - Traffic");
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->setWindowState(Qt::WindowMaximized);
    win->show();
  } else {
    constexpr size_t kGridW = 90;
    constexpr size_t kGridH = 33;

    std::string script =
        MakeScript({{7, 6}, {20, 7}, {37, 6}, {5, 24}, {22, 23}, {52, 6}});

    mDiseaseWorld = std::make_unique<InfectiousWorld>(kGridW, kGridH, script);
    WorldGrid &grid = mDiseaseWorld->GetGrid();
    size_t wall = mDiseaseWorld->GetWallID();
    size_t floor = mDiseaseWorld->GetFloorID();

    // outer border
    for (size_t y = 0; y < kGridH; ++y) {
      grid[0, y] = wall;
      grid[kGridW - 1, y] = wall;
    }
    for (size_t x = 0; x < kGridW; ++x) {
      grid[x, 0] = wall;
      grid[x, kGridH - 1] = wall;
    }

    // buildings copied from Group13_main.cpp
    // Wells Hall
    DrawBuilding(grid, wall, floor, 2, 2, 13, 11, {{7, 11}, {13, 6}});

    // MSU Union
    DrawBuilding(grid, wall, floor, 17, 2, 29, 11,
                 {{17, 6}, {23, 11}, {29, 6}, {23, 2}});

    // Berkey Hall
    DrawBuilding(grid, wall, floor, 33, 2, 42, 11, {{33, 6}, {38, 11}});

    // Shaw Hall
    DrawBuilding(grid, wall, floor, 48, 2, 60, 11, {{48, 6}, {54, 11}});

    // Library
    DrawBuilding(grid, wall, floor, 64, 2, 76, 11,
                 {{64, 6}, {70, 11}, {76, 6}});

    // Natural Science
    DrawBuilding(grid, wall, floor, 80, 2, 88, 11, {{80, 6}, {84, 11}});

    // Red Cedar River
    for (size_t y = 14; y <= 16; ++y)
      for (size_t x = 1; x <= kGridW - 2; ++x) grid[x, y] = wall;
    // bridges
    for (size_t y = 14; y <= 16; ++y) {
      for (size_t x = 7; x <= 10; ++x) grid[x, y] = floor;
      for (size_t x = 43; x <= 46; ++x) grid[x, y] = floor;
      for (size_t x = 69; x <= 72; ++x) grid[x, y] = floor;
    }

    // EB
    DrawBuilding(grid, wall, floor, 2, 19, 15, 28,
                 {{9, 19}, {15, 24}, {9, 28}});

    // Brody
    DrawBuilding(grid, wall, floor, 19, 19, 32, 27, {{26, 19}, {19, 23}});

    // Stadium
    DrawBuilding(grid, wall, floor, 36, 19, 57, 30,
                 {{47, 19}, {36, 24}, {47, 30}, {57, 24}});

    // quarantine zone (Olin Health Center)
    mDiseaseWorld->AddQuarantineZone(
        Box::FromCorners(Point(77.0, 18.0), Point(89.0, 30.0)));

    // disease parameters
    mDiseaseWorld->SetTransmissionRate(0.50);
    mDiseaseWorld->SetInfectionRadius(2.5);
    mDiseaseWorld->SetTreatmentDuration(40);
    mDiseaseWorld->SetImmunityDuration(50);
    mDiseaseWorld->SetFallbackRecoveryTicks(100);
    mDiseaseWorld->SetClinicEntrance(WorldPosition{82, 24});
    mDiseaseWorld->SetRecoveryExit(WorldPosition{70, 17});

    // swarming agents
    mDiseaseWorld->AddAgent<SwarmingAgent<DiseaseData>>(
        DiseaseData{WorldPosition{14, 12}});
    mDiseaseWorld->AddAgent<SwarmingAgent<DiseaseData>>(
        DiseaseData{WorldPosition{44, 12}});
    mDiseaseWorld->AddAgent<SwarmingAgent<DiseaseData>>(
        DiseaseData{WorldPosition{68, 12}});
    mDiseaseWorld->AddAgent<SwarmingAgent<DiseaseData>>(
        DiseaseData{WorldPosition{16, 17}});
    mDiseaseWorld->AddAgent<SwarmingAgent<DiseaseData>>(
        DiseaseData{WorldPosition{44, 17}});
    mDiseaseWorld->AddAgent<SwarmingAgent<DiseaseData>>(
        DiseaseData{WorldPosition{62, 17}});
    mDiseaseWorld->AddAgent<SwarmingAgent<DiseaseData>>(
        DiseaseData{WorldPosition{47, 19}});

    // patient zero
    mDiseaseWorld->InfectAgent(6);

    /*
    auto *win = new InfectiousWindow(*mDiseaseWorld, mImagePaths, mTileSize,
                                     mAgentImagePath);
                                     */
    auto *win = new MainWindow(*mDiseaseWorld, mImagePaths, mTileSize,
                               mAgentImagePath, nullptr, 2);
    win->mOwnedWorld = std::move(mDiseaseWorld);
    win->setWindowTitle("Group 21 Demo - Virus");
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->setWindowState(Qt::WindowMaximized);
    win->show();
  }

  close();
}

}  // namespace cse498