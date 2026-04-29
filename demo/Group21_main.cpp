#include <QApplication>

#include "../source/Interfaces/gui/StartScreen.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("Group 21 Demo");
  app.setApplicationVersion("1.0");

  const std::vector<QString> backgroundImagePaths = {"images/test1.png",
                                                     "images/test2.png"};
  const QString agentImagePath = "images/agent.png";
  const int tileSize = 20;

  cse498::StartScreen startScreen(backgroundImagePaths, tileSize,
                                  agentImagePath);

  // startScreen.setWindowState(Qt::WindowMaximized);
  startScreen.show();

  return app.exec();
}
