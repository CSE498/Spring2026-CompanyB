#include <QApplication>
#include "StartScreen.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Group 21 Demo");
    app.setApplicationVersion("1.0");

    const std::vector<QString> backgroundImagePaths = {    "images/test1.png",  // Unknown
    "images/test1.png",  // road
    "images/test2.png",  // grass
    "images/test1.png",  // traffic_light_vertical
    "images/test1.png",  // traffic_light_horizontal
    "images/test1.png",  // spawn
    "images/test2.png",  // destination};
     };
    const int tileSize = 64;
    const QString agentImagePath = "images/agent.png";

    cse498::StartScreen startScreen(backgroundImagePaths, tileSize, agentImagePath);
    startScreen.show();

    return app.exec();
}