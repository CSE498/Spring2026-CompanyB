# references: https://doc.qt.io/qt-6/cmake-get-started.html
#             https://somcosoftware.com/en/blog/how-to-use-qt-and-qml-with-visual-studio-code-and-wsl#writing-a-simple-qml-app

cmake_minimum_required(VERSION 3.16)

project(QtApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Widgets)


qt_add_executable(QtApp
        source/Interfaces/gui/MadiQTtest.cpp
        source/Interfaces/gui/ImageManager.cpp
        source/Interfaces/gui/ImageGrid.cpp
        source/Interfaces/gui/MainWindow.cpp
        source/Interfaces/gui/StartScreen.cpp
)

target_link_libraries(QtApp PRIVATE Qt6::Core Qt6::Widgets)