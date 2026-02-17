#include <cassert>
#include <iostream>
#include "../source/Interfaces/WebButton.h"

int main() {
  WebButton b("Start", "startBtn");

  // Label test
  assert(b.GetLabel() == "Start");
  b.SetLabel("Pause");
  assert(b.GetLabel() == "Pause");

  // Visibility test
  assert(b.IsVisible() == true);
  b.Hide();
  assert(b.IsVisible() == false);
  b.Show();
  assert(b.IsVisible() == true);

  // Enable/Disable test
  assert(b.IsEnabled() == true);
  b.Disable();
  assert(b.IsEnabled() == false);
  b.Enable();
  assert(b.IsEnabled() == true);

  // Click callback test
  int count = 0;
  b.SetOnClick([&]() { count++; });

  b.Click();
  assert(count == 1);

  b.Disable();
  b.Click();
  assert(count == 1);

  b.Enable();
  b.Hide();
  b.Click();
  assert(count == 1);

  b.Show();
  b.Click();
  assert(count == 2);

  std::cout << "All WebButton tests passed!\n";
  return 0;
}
