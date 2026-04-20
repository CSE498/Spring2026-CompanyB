#include <QApplication>
#include "StartScreen.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Group 21 Demo");
    app.setApplicationVersion("1.0");

    const std::vector<QString> backgroundImagePaths = {
    "images/road_temp.png",  // road
    "images/grass_temp.png",  // grass
    "images/traffic_light_v_temp.png",  // traffic_light_vertical
    "images/traffic_light_h_temp.png",  // traffic_light_horizontal
    "images/spawn_temp.png",  // spawn
    "images/destination_temp.png",  // destination};
     };
    const int tileSize = 15;
    const QString agentImagePath = "images/agent.png";

    cse498::StartScreen startScreen(backgroundImagePaths, tileSize, agentImagePath);
    startScreen.show();

    return app.exec();
}