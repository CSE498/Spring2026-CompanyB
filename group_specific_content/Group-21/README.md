## Build Notes

#### ImageGrid
CMake instructions:

WSL:
1. mkdir build
2. cd build
3. cmake ..
4. cmake --build .
5. cd ..
6. ./build/QtApp

Windows:
1. mkdir build
2. cd build
3. cmake -G "MinGW Makefiles" .. 
    (The -G option specifies which generator to use, and to my knowledge everyone in our group that is on Windows is using MinGW. Without it, CMake defaults to MSVC and causes a bunch of linker errors and this was the only solution I found.)
4. cmake --build .
5. cd ..
6. ./build/QtApp.exe

Mac:
- TBD

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