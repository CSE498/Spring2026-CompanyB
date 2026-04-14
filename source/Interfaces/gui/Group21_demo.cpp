#include <QApplication>
#include "MainWindow.hpp"
#include "../source/Worlds/MazeWorld.hpp"
#include "../source/Worlds/InfectiousWorld.hpp"
#include "../source/Agents/PacingAgent.hpp"

#include "StartScreen.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Group 21 Demo");
    app.setApplicationVersion("1.0");

    // cse498::MazeWorld world;
    cse498::InfectiousWorld world;

    const std::vector<QString> backgroundImagePaths = {"images/test1.png", "images/test2.png"};
    const QString agentImagePath = "images/agent.png";
    const int tileSize = 64;

    // Disease parameters
    world.SetTransmissionRate(0.4);
    world.SetInfectionRadius(1.5);
    world.SetInfectionDuration(8);
    world.SetImmunityDuration(15);

    // Agents
    world.AddAgent<cse498::PacingAgent>("Agent-1").SetLocation(cse498::WorldPosition{3, 2});
    world.AddAgent<cse498::PacingAgent>("Agent-2").SetLocation(cse498::WorldPosition{5, 4});
    world.AddAgent<cse498::PacingAgent>("Agent-3").SetLocation(cse498::WorldPosition{7, 3});
    world.AddAgent<cse498::PacingAgent>("Agent-4").SetLocation(cse498::WorldPosition{4, 6});
    world.AddAgent<cse498::PacingAgent>("Agent-5").SetLocation(cse498::WorldPosition{8, 7});
    world.AddAgent<cse498::PacingAgent>("Agent-6").SetLocation(cse498::WorldPosition{12, 3});
    world.AddAgent<cse498::PacingAgent>("Agent-7").SetLocation(cse498::WorldPosition{15, 5});
    world.AddAgent<cse498::PacingAgent>("Agent-8").SetLocation(cse498::WorldPosition{14, 2});
    world.AddAgent<cse498::PacingAgent>("Agent-9").SetLocation(cse498::WorldPosition{16, 7});
    world.AddAgent<cse498::PacingAgent>("Agent-10").SetLocation(cse498::WorldPosition{13, 7});

    // Patient zero!
    world.InfectAgent(0);

    cse498::MainWindow startScreen(world, backgroundImagePaths, tileSize, agentImagePath);
    startScreen.show();

    return app.exec();
}