/**
 * @file RobinHoodMap.hpp
 * @brief Declaration of the RobinHoodMap class.
 * @author John Stouffer
 * @date 2026-2-2
 */

#pragma once

#include <functional>
#include <vector>

// forward declaration for testing RobinHoodMap
class RobinHoodMapTest;

/**
 * @brief A hash map implementation using the Robin Hood hashing algorithm.
 */
template <typename K, typename V>
class RobinHoodMap {
 private:
  struct Entry {
    K key;                 //< The key used to access the entry
    V value;               //< The value associated with the key
    size_t hash = 0;       //< Stored hash to avoid recomputation
    bool filled = false;   //< Indicates if the entry is filled
  };

  /// @brief the physical table storing entries
  std::vector<Entry> mTable{8, Entry{}};
  /// @brief the number of filled entries
  size_t mSize = 0;
  /// @brief the hash function we use
  std::hash<K> mHasher;

  /**
   * @brief struct return when looking for a key
   */
  struct KeyResult {
    bool worked;
    K& key;
  };

 public:
 /**
  * @brief struct return when looking for a value
  */
  struct ValueResult {
    bool worked;
    V& value;
  };

 private:
 /**
  * @brief Finds the first filled element in the table.
  * @return KeyResult containing success flag and reference to key.
  */
  KeyResult _findFirstElement() {
    for (size_t i = 0; i < mTable.size(); ++i) {
      if (mTable[i].filled) {
        return {true, mTable[i].key};
      }
    }
    return {false, mTable[0].key};
  }

  /**
   * @brief Resizes the internal table to double its current size.
   * Rehashes and reinserts all existing elements.
   */
  void _resize() {
    std::vector<Entry> oldTable = std::move(mTable);
    mTable = std::vector<Entry>(oldTable.size() * 2, Entry{});
    mSize = 0;

    for (auto& entry : oldTable) {
      if (entry.filled) {
        _insertWithHash(std::move(entry.key), std::move(entry.value), entry.hash);
      }
    }
  }

  /// @brief Internal insert that reuses precomputed hash
  void _insertWithHash(K key, V value, size_t hash) {
    const size_t mask = mTable.size() - 1;
    size_t index = hash & mask;
    size_t probeCount = 0;

    while (mTable[index].filled) {
      if (mTable[index].key == key) {
        mTable[index].value = std::move(value);
        return;
      }

      size_t existingProbeCount = (index - (mTable[index].hash & mask) + mTable.size()) & mask;

      if (existingProbeCount < probeCount) {
        std::swap(key, mTable[index].key);
        std::swap(value, mTable[index].value);
        std::swap(hash, mTable[index].hash);
        probeCount = existingProbeCount;
      }

      ++probeCount;
      index = (index + 1) & mask;
    }

    mTable[index].key = std::move(key);
    mTable[index].value = std::move(value);
    mTable[index].hash = hash;
    mTable[index].filled = true;
    ++mSize;
  }

 public:
  /// @brief Default constructor and destructor
  RobinHoodMap() = default;
  ~RobinHoodMap() = default;

  void insert(const K& key, const V& value);
  void remove(const K& key);
  ValueResult at(const K& key) const;
  ValueResult operator[](const K& key) const;
  size_t size() const;
  void operator=(const RobinHoodMap<K, V>& other);

  /**
   * @brief Iterator class for RobinHoodMap entries.
   */
  class Iterator {
   private:
    Entry* mEntry; //< Current entry pointer
    Entry* mEnd;   //< End entry pointer

   public:
    /// @brief Constructs an iterator pointing to a specific entry.
    /// @param ptr Pointer to the current entry.
    /// @param end Pointer to the end entry.
    explicit Iterator(Entry* ptr, Entry* end = nullptr)
        : mEntry(ptr), mEnd(end) {}

    /// @brief Dereferences the iterator to access the current entry.
    /// @return Reference to the current entry.
    Entry& operator*() { return *mEntry; }

    /// @brief Pre-increment operator to move to the next filled entry.
    /// @return the updated iterator.
    Iterator& operator++() {
      ++mEntry;
      while (mEntry != mEnd && !mEntry->filled) {
        ++mEntry;
      }
      return *this;
    }

    /// @brief Pre-decrement operator to move to the previous entry.
    /// @return the updated iterator.
    Iterator& operator--() {
      --mEntry;
      return *this;
    }

    /// @brief Post-increment operator to move to the next filled entry.
    /// @return the original iterator before increment.
    Iterator operator++(int) {
      Iterator temp = *this;
      ++(*this);
      return temp;
    }

    /// @brief Post-decrement operator to move to the previous entry.
    /// @return the original iterator before decrement.
    Iterator operator--(int) {
      Iterator temp = *this;
      --(*this);
      return temp;
    }

    /**
     * @brief Inequality operator to compare two iterators.
     * @param other The other iterator to compare with.
     * @return true if the iterators are not equal, false otherwise.
     */
    bool operator!=(const Iterator& other) const {
      return mEntry != other.mEntry;
    }

    /**
     * @brief Equality operator to compare two iterators.
     * @param other The other iterator to compare with.
     * @return true if the iterators are equal, false otherwise.
     */
    bool operator==(const Iterator& other) const {
      return mEntry == other.mEntry;
    }

    /**
     * @brief Addition operator to advance the iterator by an offset.
     * @param offset The number of positions to advance.
     * @return A new iterator advanced by the specified offset.
     */
    Iterator operator+(size_t offset) const {
      return Iterator(mEntry + offset, mEnd);
    }

    /**
     * @brief Subtraction operator to move the iterator backward by an offset.
     * @param offset The number of positions to move backward.
     * @return A new iterator moved backward by the specified offset.
     */
    Iterator operator-(size_t offset) const {
      return Iterator(mEntry - offset, mEnd);
    }
  };

  /**
   * @brief Returns an iterator to the first filled entry in the map.
   * @return Iterator pointing to the first filled entry.
   */
  Iterator begin() {
    for (size_t i = 0; i < mTable.size(); ++i) {
      if (mTable[i].filled) {
        return Iterator(&mTable[i], &mTable[mTable.size()]);
      }
    }
    return end();
  }

  /**
   * @brief Returns an iterator to one past the last entry in the map.
   * @return Iterator pointing to one past the last entry.
   */
  Iterator end() {
    return Iterator(&mTable[mTable.size()], &mTable[mTable.size()]);
  }

  friend class RobinHoodMapTest;
};