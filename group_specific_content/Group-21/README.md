## Build Notes

#### ImageGrid
On WSL with Qt installed, use cmake and run ./build/QtApp.

(Previously used this at and before commit 793df6d3ed51bd34fcccee51591ce4bd4635fe1d, but it only worked on my PC and not my laptop, and a peer review had the same problem)
g++ -std=c++23  ./source/Interfaces/gui/MadiQtTest.cpp     -o TestingQt     -I./third-party/qt/include     -L./third-party/qt/lib     -lQt6Core     -lQt6Gui     -lQt6Widgets     -fPIC