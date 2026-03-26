/**
 * @file DataMap.hpp
 * @brief Declaration of the DataMap class
 * @author Cindy Huang
 * @date 2026-02-05
 */

#pragma once

#include <any>
#include <cassert>
#include <expected>
#include <string>
#include <unordered_map>

namespace cse498 {

class DataMap {
 private:
  using MapType = std::unordered_map<std::string, std::any>;
  MapType mData;

 public:
  /// Iterator class that wraps the underlying unordered_map iterator.
  class Iterator {
   private:
    /// Iterator to the underlying map
    MapType::iterator mIt;

   public:
    using reference = typename MapType::iterator::reference;
    using pointer = typename MapType::iterator::pointer;

    Iterator() = default;
    explicit Iterator(MapType::iterator it) : mIt(it) {}

    /**
     * Dereference operator returns a reference to the underlying
     * std::pair<const std::string, std::any>.
     */
    reference operator*() const { return *mIt; }
    /**
     * Arrow operator overload
     */
    pointer operator->() const { return &(*mIt); }

    /**
     * Pre-increment and post-increment operators to advance the iterator.
     */
    Iterator& operator++() {
      ++mIt;
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++mIt;
      return tmp;
    }

    /**
     * Equality and inequality operators to compare iterators.
     */
    bool operator==(const Iterator& other) const { return mIt == other.mIt; }
    bool operator!=(const Iterator& other) const { return mIt != other.mIt; }

    /// Returns the key of the current element.
    const std::string& Key() const { return mIt->first; }

    /// Returns the std::any value of the current element.
    std::any& Value() { return mIt->second; }
    const std::any& Value() const { return mIt->second; }

    /// Convenience: cast the value to the specified type.
    template <typename V>
    V As() const {
      return std::any_cast<V>(mIt->second);
    }

    /// Convenience: get a reference to the stored value of the specified type.
    template <typename V>
    V& AsRef() {
      return *std::any_cast<V>(&mIt->second);
    }
  };

  /// Const iterator class that wraps the underlying unordered_map
  /// const_iterator.
  class ConstIterator {
   private:
    /// Const iterator to the underlying map
    MapType::const_iterator mIt;

   public:
    using reference = typename MapType::const_iterator::reference;
    using pointer = typename MapType::const_iterator::pointer;

    ConstIterator() = default;
    explicit ConstIterator(MapType::const_iterator it) : mIt(it) {}

    /**
     * Dereference operator returns a reference to the underlying
     * std::pair<const std::string, std::any>.
     */
    reference operator*() const { return *mIt; }
    /**
     * Arrow operator overload
     */
    pointer operator->() const { return &(*mIt); }

    /**
     * Pre-increment and post-increment operators to advance the iterator.
     */
    ConstIterator& operator++() {
      ++mIt;
      return *this;
    }
    ConstIterator operator++(int) {
      ConstIterator tmp = *this;
      ++mIt;
      return tmp;
    }

    /**
     * Equality and inequality operators to compare iterators.
     */
    bool operator==(const ConstIterator& other) const {
      return mIt == other.mIt;
    }
    bool operator!=(const ConstIterator& other) const {
      return mIt != other.mIt;
    }

    /**
     * Returns the key of the current element.
     */
    const std::string& Key() const { return mIt->first; }
    /**
     * Returns the std::any value of the current element.
     */
    const std::any& Value() const { return mIt->second; }

    /**
     * Convenience: cast the value to the specified type.
     */
    template <typename V>
    V As() const {
      return std::any_cast<V>(mIt->second);
    }
  };

  Iterator begin() {
    return Iterator(mData.begin());
  }  //< Returns an iterator to the beginning of the map
  Iterator end() {
    return Iterator(mData.end());
  }  //< Returns an iterator to the end of the map
  ConstIterator begin() const {
    return ConstIterator(mData.cbegin());
  }  //< Returns a const iterator to the beginning of the map
  ConstIterator end() const {
    return ConstIterator(mData.cend());
  }  //< Returns a const iterator to the end of the map
  ConstIterator cbegin() const {
    return ConstIterator(mData.cbegin());
  }  //< Returns a const iterator to the beginning of the map
  ConstIterator cend() const {
    return ConstIterator(mData.cend());
  }  //< Returns a const iterator to the end of the map

  /**
   * Map-style access. Inserts an empty std::any when the key does not exist.
   * @param name The key to access.
   * @return A mutable reference to the stored std::any value.
   */
  std::any& operator[](const std::string& name) {
    if (mData.find(name) == mData.end()) {
      mData[name] = std::any();
    }
    return mData[name];
  }

  /**
   * Const map-style access.
   * @param name The key to access.
   * @return A const reference to the stored std::any value.
   */
  [[nodiscard]] const std::any& operator[](const std::string& name) const {
    auto it = mData.find(name);
    assert(it != mData.end() && "DataMap::operator[] const: key not found");
    return it->second;
  }

  /**
   * Associates the given value with the specified key. If the key already
   * exists, its value is overwritten.
   * @tparam V The type of the value to store.
   * @param name The key to associate with the value.
   * @param value The value to store in the map.
   */
  template <typename V>
  void Set(const std::string& name, const V& value) {
    mData[name] = value;
  }

  /**
   * Returns the value associated with the given key if it exists and can be
   * cast to the specified type; otherwise, returns an error message.
   * @tparam V The expected type of the value.
   * @param name The key associated with the value.
   * @return The value if the key exists and the type matches; otherwise, an
   * error
   */
  template <typename V>
  [[nodiscard]] std::expected<V, std::string> Get(
      const std::string& name) const {
    auto it = mData.find(name);
    if (it == mData.end()) {
      return std::unexpected("Key not found");
    }
    if (it->second.type() != typeid(V)) {
      return std::unexpected("Type mismatch for key");
    }
    return std::any_cast<V>(it->second);
  }

  /**
   * Returns a reference to the stored value of the specified type.
   * @tparam V The expected type of the value.
   * @param name The key associated with the value.
   * @return A reference to the value if the key exists and the type matches;
   * otherwise
   */
  template <typename V>
  [[nodiscard]] V& GetRef(const std::string& name) {
    auto it = mData.find(name);
    assert(
        it != mData.end() &&
        "DataMap::GetRef(): key not found (check for typo or initialization)");
    auto* p = std::any_cast<V>(&it->second);
    assert(p != nullptr &&
           "DataMap::GetRef(): stored value cannot be cast to requested type");
    return *p;
  }

  /**
   * Checks if the map contains a key.
   * @param name The key to check for.
   * @return true if the key exists, false otherwise.
   */
  [[nodiscard]] bool Contains(const std::string& name) const noexcept {
    return mData.find(name) != mData.end();
  }
  /**
   * Checks if the map is empty.
   * @return true if the map is empty, false otherwise.
   */
  [[nodiscard]] bool IsEmpty() const noexcept { return mData.empty(); }
  /**
   * Returns the number of key-value pairs in the map.
   * @return The size of the map.
   */
  [[nodiscard]] int Size() const noexcept { return mData.size(); }
  /**
   * Removes all key-value pairs from the map, leaving it empty.
   */
  void Clear() noexcept { mData.clear(); }
};
}  // namespace cse498