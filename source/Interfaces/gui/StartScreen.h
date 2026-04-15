#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "MainWindow.hpp"
#include "WorldBase.hpp"
#include "../../Worlds/MazeWorld.hpp"
#include "../../Worlds/InfectiousWorld.hpp"
#include "../source/Agents/PacingAgent.hpp"
#include "../../core/WorldBase.hpp"

namespace cse498 {

class StartScreen : public QWidget {
    Q_OBJECT

public:
    explicit StartScreen(const std::vector<QString>& imagePaths,
                         int tileSize, const QString& agentImagePath,
                         QWidget* parent = nullptr);


private slots:
    void onTrafficClicked();
    void onVirusClicked();

private:
    QString mAgentImagePath{};
    void launchMainWindow(int mode);

    std::vector<QString> mImagePaths{};
    int mTileSize{};

    // owned here so it outlives MainWindow
    std::unique_ptr<WorldBase> mWorld{};

    QPushButton* mTrafficBtn{};
    QPushButton* mVirusBtn{};
    QPushButton* mGearBtn{};
};

} // namespace cse498