#include "weighted_set.hpp"

#include <iostream>
#include <map>
#include <string>
/*
 NOTE: this is a temporary test file written by Claude Code. I plan on replacing it with a proper unit test
 suite later--but for now I do need a quick way to test weighted_set.hpp
*/
int main() {
  std::cout << "=== WeightedSet Basic Tests ===" << std::endl;

  // Test 1: Empty set
  std::cout << "\n--- Test 1: Empty set ---" << std::endl;
  cse498::WeightedSet<std::string> ws;
  std::cout << "size(): " << ws.size() << " (expected 0)" << std::endl;
  std::cout << "empty(): " << (ws.empty() ? "true" : "false")
            << " (expected true)" << std::endl;
  std::cout << "total_weight(): " << ws.total_weight() << " (expected 0)"
            << std::endl;
  auto result = ws.getRandomElement();
  std::cout << "getRandomElement() on empty: "
            << (result.has_value() ? "got value (BAD!)" : "nullopt (good)")
            << std::endl;

  // Test 2: Insert single element
  std::cout << "\n--- Test 2: Insert single element ---" << std::endl;
  bool inserted = ws.insert("apple", 1.0);
  std::cout << "insert(\"apple\", 1.0): " << (inserted ? "true" : "false")
            << " (expected true)" << std::endl;
  std::cout << "total_weight(): " << ws.total_weight() << " (expected 1.0)"
            << std::endl;

  // Test 3: getElementAt on single element
  std::cout << "\n--- Test 3: getElementAt on single element ---" << std::endl;
  auto elem = ws.getElementAt(0.5);
  std::cout << "getElementAt(0.5): "
            << (elem.has_value() ? elem.value() : "nullopt")
            << " (expected apple)" << std::endl;

  // Test 4: getRandomElement on single element
  std::cout << "\n--- Test 4: getRandomElement (single element) ---"
            << std::endl;
  auto random_elem = ws.getRandomElement();
  std::cout << "getRandomElement(): "
            << (random_elem.has_value() ? random_elem.value() : "nullopt")
            << " (expected apple)" << std::endl;

  // Test 5: Insert with invalid weight
  std::cout << "\n--- Test 5: Insert with invalid weight ---" << std::endl;
  bool bad_insert1 = ws.insert("bad", 0.0);
  bool bad_insert2 = ws.insert("bad", -1.0);
  std::cout << "insert(\"bad\", 0.0): " << (bad_insert1 ? "true" : "false")
            << " (expected false)" << std::endl;
  std::cout << "insert(\"bad\", -1.0): " << (bad_insert2 ? "true" : "false")
            << " (expected false)" << std::endl;

  // Test 6: Insert multiple elements
  std::cout << "\n--- Test 6: Insert multiple elements ---" << std::endl;
  cse498::WeightedSet<std::string> ws2;
  ws2.insert("a", 1.0);
  ws2.insert("b", 2.0);
  ws2.insert("c", 3.0);
  std::cout << "Inserted a(1.0), b(2.0), c(3.0)" << std::endl;
  std::cout << "total_weight(): " << ws2.total_weight() << " (expected 6.0)"
            << std::endl;

  // Test 7: getElementAt boundary conditions
  std::cout << "\n--- Test 7: getElementAt boundary conditions ---"
            << std::endl;
  auto at_neg = ws2.getElementAt(-0.1);
  auto at_over = ws2.getElementAt(6.1);
  std::cout << "getElementAt(-0.1): "
            << (at_neg.has_value() ? at_neg.value() : "nullopt")
            << " (expected nullopt)" << std::endl;
  std::cout << "getElementAt(6.1): "
            << (at_over.has_value() ? at_over.value() : "nullopt")
            << " (expected nullopt)" << std::endl;

  // Test 8: Random distribution check
  std::cout << "\n--- Test 8: Random distribution (rough check) ---"
            << std::endl;
  std::cout
      << "Getting 1000 random elements from set with a(1), b(2), c(3)..."
      << std::endl;
  std::map<std::string, int> counts;
  for (int i = 0; i < 1000; i++) {
    auto r = ws2.getRandomElement();
    if (r.has_value()) {
      counts[r.value()]++;
    }
  }
  std::cout << "Distribution:" << std::endl;
  for (const auto& [key, count] : counts) {
    double percent = count / 10.0;
    std::cout << "  " << key << ": " << count << " (" << percent
              << "%, expected ~"
              << (key == "a" ? "16.7" : key == "b" ? "33.3" : "50.0") << "%)"
              << std::endl;
  }

  // Test 9: Integer type
  std::cout << "\n--- Test 9: Integer type ---" << std::endl;
  cse498::WeightedSet<int> ws_int;
  ws_int.insert(100, 5.0);
  ws_int.insert(200, 5.0);
  auto int_elem = ws_int.getRandomElement();
  std::cout << "getRandomElement() from int set: "
            << (int_elem.has_value() ? std::to_string(int_elem.value())
                                     : "nullopt")
            << " (expected 100 or 200)" << std::endl;
  std::cout << "total_weight(): " << ws_int.total_weight() << " (expected 10.0)"
            << std::endl;

  std::cout << "\n=== Tests Complete ===" << std::endl;
  return 0;
}
