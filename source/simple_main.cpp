/**
* @file simple_main.cpp
 * @brief Visual Demo of WebTextbox functionality
 */

#include "Interfaces/WebTextbox.hpp"
#include <string>

using namespace cse498;

int main() {
    // 1. MAIN LOG BOX (Blue)
    // 'static' prevents the destructor from running when main() exits.
    static WebTextbox logBox("main_log");

    TextStyle logStyle;
    logStyle.color = "#003366"; // Dark Blue
    logStyle.backgroundColor = "#e6f2ff"; // Light Blue
    logStyle.fontSize = 14;
    logStyle.fontFamily = "monospace";
    logStyle.bold = true;

    logBox.SetStyle(logStyle);
    logBox.SetPosition(20, 20);
    logBox.SetSize(400, 200);

    logBox.SetText(" [SYSTEM STARTUP] \n");
    logBox.AppendText("--------------------------------\n");
    logBox.AppendText(" > WebTextbox initialized.\n");
    logBox.AppendText(" > Static memory allocation active.\n");
    logBox.AppendText(" > Waiting for simulation data...\n");

    // 2. STATUS INDICATOR (Green Badge)
    static WebTextbox statusBox("status_badge");
    TextStyle statusStyle;
    statusStyle.color = "white";
    statusStyle.backgroundColor = "#28a745"; // Green
    statusStyle.bold = true;

    statusBox.SetStyle(statusStyle);
    statusBox.SetPosition(440, 20);
    statusBox.SetSize(120, 40);
    statusBox.SetText(" ONLINE ");

    // 3. WARNING BOX (Red - showing text wrapping)
    static WebTextbox alertBox("alert_box");
    TextStyle alertStyle;
    alertStyle.color = "#721c24";
    alertStyle.backgroundColor = "#f8d7da";

    alertBox.SetStyle(alertStyle);
    alertBox.SetPosition(20, 240);
    alertBox.SetSize(300, 100);
    alertBox.SetText("WARNING: No simulation agents detected. Please load Agent Modules to begin traffic simulation.");

    return 0;
}





// /**
//  * This file is part of the Fall 2026, CSE 498, section 2, course project.
//  * @brief A simplistic main file to demonstrate a system.
//  * @note Status: PROPOSAL
//  **/
//
// // Include the modules that we will be using.
// #include "Agents/PacingAgent.hpp"
// #include "Interfaces/TrashInterface.hpp"
// #include "Worlds/MazeWorld.hpp"
//
// using namespace cse498;
//
// int main() {
//   MazeWorld world;
//   world.AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3, 1});
//   world.AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6, 1});
//   world.AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(
//       WorldPosition{7, 7});
//   world.AddAgent<PacingAgent>("Guard 2")
//       .SetHorizontal()
//       .ToggleDirection()
//       .SetLocation(WorldPosition{8, 8});
//   world.AddAgent<TrashInterface>("Interface")
//       .SetSymbol('@')
//       .SetLocation(WorldPosition{1, 1});
//
//   world.Run();
// }
