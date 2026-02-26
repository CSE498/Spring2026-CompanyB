#include "../../source/tools/DataFileManager.hpp"
#include "catch2/catch.hpp"
#include <string>
#include <vector>

/**
 * Helper to register a set of simple attributes for testing.
 */
void RegisterTestAttributes(cse498::DataFileManager& dfm, int count) {
  for (int i = 0; i < count; ++i) {
    std::string name = "Col_" + std::to_string(i);
    dfm.RegisterAttribute(name, static_cast<std::function<int()>>([i]() { return i; }));
  }
}

TEST_CASE("DataFileManager Registration and State", "[data_file_manager]") {
  cse498::DataFileManager dfm;

  SECTION("initial state") {
    CHECK(dfm.GetRowCount() == 0);
    CHECK(dfm.GetColCount() == 0);
  }

  SECTION("registration tracking") {
    std::function<int()> Get42 = []() { return 42; };
    dfm.RegisterAttribute("IntCol", Get42);
    CHECK(dfm.GetColCount() == 1);

    dfm.RegisterAttribute("DoubleCol", static_cast<std::function<double()>>([]() { return 3.14; }));
    dfm.RegisterAttribute("StringCol", static_cast<std::function<std::string()>>([]() { return std::string("test"); }));
    CHECK(dfm.GetColCount() == 3);
  }
}

TEST_CASE("DataFileManager Update and Buffering", "[data_file_manager]") {
  cse498::DataFileManager dfm;

  SECTION("update without attributes") {
    dfm.Update();
    CHECK(dfm.GetRowCount() == 0);
  }

  SECTION("single row buffering") {
    int val = 10;
    std::function<int()> GetVal = [&val]() { return val; };
    dfm.RegisterAttribute("Value", GetVal);
    
    dfm.Update();
    CHECK(dfm.GetRowCount() == 1);
    
    val = 20;
    dfm.Update();
    CHECK(dfm.GetRowCount() == 2);
  }

  SECTION("multiple columns and rows") {
    RegisterTestAttributes(dfm, 5);
    
    for (int i = 0; i < 10; ++i) {
      dfm.Update();
    }
    
    CHECK(dfm.GetColCount() == 5);
    CHECK(dfm.GetRowCount() == 10);
  }

  SECTION("clearing the buffer") {
    RegisterTestAttributes(dfm, 2);
    dfm.Update();
    dfm.Update();
    REQUIRE(dfm.GetRowCount() == 2);

    dfm.Clear();
    CHECK(dfm.GetRowCount() == 0);
    // Columns should remain even if data is cleared
    CHECK(dfm.GetColCount() == 2);
  }
}

TEST_CASE("DataFileManager File I/O", "[data_file_manager]") {
  cse498::DataFileManager dfm;

  SECTION("saving empty manager") {
    // Should fail or at least handle gracefully if no attributes exist
    bool success = dfm.SaveToDisk("test_empty.csv");
    CHECK(success);
    // Since SaveToDisk opens a file, it might return true but result in an empty file.
    // However, our logic returns false if filename is empty.
    CHECK_FALSE(dfm.SaveToDisk(""));
    std::remove("test_empty.csv");
  }

  SECTION("buffer reset after save") {
    dfm.RegisterAttribute("Data", static_cast<std::function<std::string()>>([]() { return "example"; }));
    dfm.Update();
    REQUIRE(dfm.GetRowCount() == 1);

    // SaveToDisk should clear the buffer on success
    if (dfm.SaveToDisk("test_output.csv")) {
      CHECK(dfm.GetRowCount() == 0);
      std::remove("test_output.csv");
    }
  }
}

TEST_CASE("DataFileManager Template Genericity", "[data_file_manager]") {
  cse498::DataFileManager dfm;

  SECTION("handling various types") {
    // This tests that the template successfully resolves different types
    // and converts them to the internal string representation.
    dfm.RegisterAttribute("Int", static_cast<std::function<int()>>([]() { return 1; }));
    dfm.RegisterAttribute("Double", static_cast<std::function<double()>>([]() { return 2.2; }));
    dfm.RegisterAttribute("Float", static_cast<std::function<float()>>([]() { return 3.3f; }));
    dfm.RegisterAttribute("String", static_cast<std::function<std::string()>>([]() { return std::string("four"); }));
    dfm.RegisterAttribute("CharPointer", static_cast<std::function<const char*()>>([]() { return "five"; }));

    dfm.Update();
    CHECK(dfm.GetColCount() == 5);
    CHECK(dfm.GetRowCount() == 1);
  }
}