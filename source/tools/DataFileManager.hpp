#ifndef DATA_FILE_MANAGER_HPP
#define DATA_FILE_MANAGER_HPP

#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <ranges>

namespace cse498 {

/**
 * Class: DataFileManager
 * A generic tool to record and buffer simulation states into a 2D structure.
 *
 * This class records values for attributes of different agents. You tell it
 * which variables to watch (RegisterAttribute), and every time you call
 * Update(), it takes a snapshot of those variables and stores them in a new row
 * of a 2D vector.
 *
 * Eventually this class will support writing to an actual CSV file or a .o
 * file, but for now it just buffers the data in memory.
 */
class DataFileManager {
 public:
  // We use strings for the buffer because it's the standard format for CSV
  // files which we will eventualy write to
  using Row = std::vector<std::string>;
  using Table = std::vector<Row>;

  DataFileManager() = default;

  /**
   * Adds a "Getter" function to the manager.
   * The manager will call this function every time Update() is triggered.
   * * Template <typename T>: Allows passing functions that return ints,
   * doubles, etc.
   */
  template <typename T>
  void RegisterAttribute(const std::string& columnName,
                         std::function<T()> getter) {
    assert(!columnName.empty() && "Column name cannot be empty");

    // Wrap the getter in a lambda that converts the result to a string
    auto stringWrapper = [getter]() -> std::string {
      if constexpr (std::is_same_v<T, std::string>) {
        return getter();
      } else {
        // idea taken from
        // https://stackoverflow.com/questions/71041798/how-to-convert-from-any-type-to-stdstring-in-c
        std::ostringstream oss{};
        oss << getter();
        return oss.str();
      }
    };

    m_columnNames.push_back(columnName);   // Stores the column name
    m_callbacks.push_back(stringWrapper);  // Stores the function as a string
  }

  /**
   * Cycles through all registered functions, collects their current values,
   * and adds a new "Row" to our internal 2D Table.
   */
  void Update() {
    if (m_callbacks.empty()) return;

    Row newRow;
    newRow.reserve(m_callbacks.size());  // Reserves memory

    for (const auto& fetchValue : m_callbacks) {
      newRow.push_back(fetchValue());
    }

    m_table.push_back(std::move(newRow));
  }

  /**
   * Exports the buffered 2D Table to a CSV file.
   * Returns true if successful, false otherwise.
   */
  [[nodiscard]] bool SaveToDisk(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return false;

    // Uses ranges for clearer looping
    for (const auto& [i, name] : std::views::enumerate(m_columnNames)) {
        file << name << (static_cast<size_t>(i) == m_columnNames.size() - 1 ? "" : ",");
    }
    file << "\n";

    // Write Data Rows from 2D Table
    for (const auto& row : m_table) {
      // Uses ranges for clearer looping
      for (const auto& [i, value] : std::views::enumerate(row)) {
          file << value << (static_cast<size_t>(i) == row.size() - 1 ? "" : ",");
      }
      file << "\n";
    }

    file.close();
    m_table.clear();  // Free memory after saving
    return true;
  }

  // Simple Getters for Testing
  size_t GetRowCount() const { return m_table.size(); }
  size_t GetColCount() const { return m_columnNames.size(); }
  void Clear() { m_table.clear(); }

 private:
  Table m_table;                           // Our 2d array
  std::vector<std::string> m_columnNames;  // List of Column names
  std::vector<std::function<std::string()>>
      m_callbacks;  // List of functions for recording data
};

}  // namespace cse498

#endif
