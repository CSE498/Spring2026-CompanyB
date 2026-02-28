## Build Notes

#### ImageGrid
On WSL with Qt installed, use cmake and run ./build/QtApp.

(Previously used this at and before commit 793df6d3ed51bd34fcccee51591ce4bd4635fe1d, but it only worked on my PC and not my laptop, and a peer review had the same problem)
g++ -std=c++23  ./source/Interfaces/gui/MadiQtTest.cpp     -o TestingQt     -I./third-party/qt/include     -L./third-party/qt/lib     -lQt6Core     -lQt6Gui     -lQt6Widgets     -fPIC

#### Text

(to preface these are the commands for running on a Windows device)

uncomment the main function in Text.cpp if you want to see how the text is displayed, however comment it out if you want to check the test cases, because it won't work if the main function isn't commented out

run the following commands to see the text display:

g++ -std=c++23 -Isource -Ithird-party/qt/include source/Interfaces/gui/Text.cpp -o TextMain -Lthird-party/qt/lib -lQt6Core -lQt6Gui -lQt6Widgets

PATH="third-party/qt/bin:$PATH" ./TextMain
--------------------------------------------------------------------
for testing the individual text class:

the code already comes with this part, however if removed for trying to test all the classes all at once and another had it, make sure to add it at the very top:
 #define CATCH_CONFIG_MAIN

terminal commands for running in windows: 
cd tests

g++ -std=c++23 -I../source -I../third-party/qt/include -I../third-party/Catch/single_include tools/TextTest.cpp ../source/Interfaces/gui/Text.cpp -o TextTest -L../third-party/qt/lib -lQt6Core -lQt6Gui -lQt6Widgets

PATH="../third-party/qt/bin:$PATH" ./TextTest