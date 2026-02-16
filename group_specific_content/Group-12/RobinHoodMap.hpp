/**
 * @file RobinHoodMap.hpp
 * Declaration of the RobinHoodMap class.
 * @author John Stouffer
 * @date 2026-2-2
 */

#pragma once

#include <functional>
#include <vector>
#include <expected>

class RobinHoodMapTest;

/**
 * A hash map implementation using the Robin Hood hashing algorithm.
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

  /// the physical table storing entries
  std::vector<Entry> mTable{8, Entry{}};
  /// the number of filled entries
  size_t mSize = 0;
  /// the hash function we use
  std::hash<K> mHasher;

  /**
  * Finds the first filled element in the table.
  * @return Expected a K value if found, otherwise an error message.
  */
  std::expected<K, std::string> _findFirstElement() {
    for (size_t i = 0; i < mTable.size(); ++i) {
      if (mTable[i].filled) {
        return mTable[i].key;
      }
    }
    return std::unexpected("No filled elements found");
  }

  /**
   * Resizes the internal table to double its current size.
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

  /// Internal insert that reuses precomputed hash
  void _insertWithHash(K key, V value, size_t hash) {
    const size_t mask = mTable.size() - 1;
    size_t index = hash & mask;
    size_t probeCount = 0;

    while (mTable[index].filled) {
      if (mTable[index].hash == hash && mTable[index].key == key) {
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
  /// Default constructor and destructor
  RobinHoodMap() = default;
  ~RobinHoodMap() = default;

  /**
   * Inserts a key-value pair into the map.
   * If the key already exists, its value is updated.
   * @param key The key to insert.
   * @param value The value associated with the key.
   */
  void insert(const K& key, const V& value) {
    if (mSize >= mTable.size() / 2) {
      _resize();
    }
    _insertWithHash(key, value, mHasher(key));
  }

  void remove(const K& key) {
    const size_t mask = mTable.size() - 1;
    size_t hash = mHasher(key);
    size_t index = hash & mask;
    size_t probeCount = 0;

    for (;;) {
      if (!mTable[index].filled) {
        return;
      }

      if (mTable[index].hash == hash && mTable[index].key == key) {
        mTable[index].filled = false;
        --mSize;

        size_t nextIndex = (index + 1) & mask;
        while (mTable[nextIndex].filled) {
          size_t nextProbeCount = (nextIndex - (mTable[nextIndex].hash & mask) + mTable.size()) & mask;

          if (nextProbeCount == 0) {
            break;
          }

          mTable[index] = std::move(mTable[nextIndex]);
          mTable[nextIndex].filled = false;

          index = nextIndex;
          nextIndex = (nextIndex + 1) & mask;
        }
        return;
      }

      size_t currentProbeCount = (index - (mTable[index].hash & mask) + mTable.size()) & mask;

      if (currentProbeCount < probeCount) {
        return;
      }

      ++probeCount;
      index = (index + 1) & mask;
    }
  }

  /**
   * Retrieves the value associated with the given key.
   * If the key is found, returns a ValueResult with worked=true and a reference to the value.
   * If the key is not found, returns worked=false and a reference to a default value.
   * @param key The key to look up.
   * @return ValueResult containing success flag and reference to value.
   */
  std::expected<V, std::string> at(const K& key) const {
    const size_t mask = mTable.size() - 1;
    const size_t hash = mHasher(key);
    const size_t homeIndex = hash & mask;
    size_t index = homeIndex;
    size_t probeCount = 0;

    for (;;) {
      const Entry& entry = mTable[index];
      
      if (!entry.filled) {
        return std::unexpected("Key not found");
      }

      size_t entryProbeCount = (index - (entry.hash & mask) + mTable.size()) & mask;
      if (entryProbeCount < probeCount) {
        return std::unexpected("Key not found");
      }

      if (entry.hash == hash && entry.key == key) {
        return entry.value;
      }

      ++probeCount;
      index = (index + 1) & mask;
    }
  }

  /**
   * Overloaded subscript operator to access values by key.
   * Behaves the same as the at() method.
   * @param key The key to look up.
   * @return ValueResult containing success flag and reference to value.
   */
  std::expected<V, std::string> operator[](const K& key) const {
    return at(key);
  }

  /**
   * Assignment operator to copy contents from another RobinHoodMap.
   * @param other The other RobinHoodMap to copy from.
   */
  void operator=(const RobinHoodMap<K, V>& other) {
    mTable = other.mTable;
    mSize = other.mSize;
  }

  /**
   * Returns the number of elements in the map.
   * @return The size of the map.
   */
  size_t size() const {
    return mSize;
  }

  /**
   * Iterator class for RobinHoodMap entries.
   */
  class Iterator {
   private:
    Entry* mEntry; //< Current entry pointer
    Entry* mEnd;   //< End entry pointer

   public:
    /// Constructs an iterator pointing to a specific entry.
    /// @param ptr Pointer to the current entry.
    /// @param end Pointer to the end entry.
    explicit Iterator(Entry* ptr, Entry* end = nullptr)
        : mEntry(ptr), mEnd(end) {}

    /// Dereferences the iterator to access the current entry.
    /// @return Reference to the current entry.
    Entry& operator*() { return *mEntry; }

    /// Pre-increment operator to move to the next filled entry.
    /// @return the updated iterator.
    Iterator& operator++() {
      ++mEntry;
      while (mEntry != mEnd && !mEntry->filled) {
        ++mEntry;
      }
      return *this;
    }

    /// Pre-decrement operator to move to the previous entry.
    /// @return the updated iterator.
    Iterator& operator--() {
      --mEntry;
      return *this;
    }

    /// Post-increment operator to move to the next filled entry.
    /// @return the original iterator before increment.
    Iterator operator++(int) {
      Iterator temp = *this;
      ++(*this);
      return temp;
    }

    /// Post-decrement operator to move to the previous entry.
    /// @return the original iterator before decrement.
    Iterator operator--(int) {
      Iterator temp = *this;
      --(*this);
      return temp;
    }

    /**
     * Inequality operator to compare two iterators.
     * @param other The other iterator to compare with.
     * @return true if the iterators are not equal, false otherwise.
     */
    bool operator!=(const Iterator& other) const {
      return mEntry != other.mEntry;
    }

    /**
     * Equality operator to compare two iterators.
     * @param other The other iterator to compare with.
     * @return true if the iterators are equal, false otherwise.
     */
    bool operator==(const Iterator& other) const {
      return mEntry == other.mEntry;
    }

    /**
     * Addition operator to advance the iterator by an offset.
     * @param offset The number of positions to advance.
     * @return A new iterator advanced by the specified offset.
     */
    Iterator operator+(size_t offset) const {
      return Iterator(mEntry + offset, mEnd);
    }

    /**
     * Subtraction operator to move the iterator backward by an offset.
     * @param offset The number of positions to move backward.
     * @return A new iterator moved backward by the specified offset.
     */
    Iterator operator-(size_t offset) const {
      return Iterator(mEntry - offset, mEnd);
    }
  };

  /**
   * Returns an iterator to the first filled entry in the map.
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
   * Returns an iterator to one past the last entry in the map.
   * @return Iterator pointing to one past the last entry.
   */
  Iterator end() {
    return Iterator(&mTable[mTable.size()], &mTable[mTable.size()]);
  }

  friend class RobinHoodMapTest;
};