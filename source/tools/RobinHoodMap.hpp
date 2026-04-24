/**
 * @file RobinHoodMap.hpp
 * Declaration of the RobinHoodMap class.
 * @author John Stouffer
 * @date 2026-2-2
 */

#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <expected>
#include <functional>
#include <type_traits>
#include <vector>

class RobinHoodMapTest;

namespace cse498 {

template <typename T>
concept HashableType = requires(T a) { std::hash<T>{}(a); };

/**
 * A hash map implementation using the Robin Hood hashing algorithm.
 */
template <typename K, typename V>
  requires std::is_default_constructible_v<K> &&
           std::is_default_constructible_v<V> && std::equality_comparable<K> &&
           HashableType<K>
class RobinHoodMap {
 public:
  /// Error types for RobinHoodMap operations
  struct RHMError {
    enum class Type {
      KeyNotFound,  ///< The requested key does not exist in the map
    };

    Type type;
    std::string message;

    RHMError(Type type, std::string message)
        : type(type), message(std::move(message)) {}
  };

 private:
  /**
   * Entry struct representing a key-value pair in the hash table,
   * along with metadata for Robin Hood hashing.
   */
  struct Entry {
    K key;                //< The key used to access the entry
    V value;              //< The value associated with the key
    size_t hash = 0;      //< Stored hash to avoid recomputation
    bool filled = false;  //< Indicates if the entry is filled
  };

  /// @brief the load factor threshold for resizing the table
  static constexpr double RHM_LOAD_FACTOR = 0.75;
  /// @brief the initial size of the hash table MUST ALWAYS BE A POWER OF 2 FOR
  /// BIT MASKING TO WORK
  static constexpr size_t INITIAL_SIZE = 8;

  /// the physical table storing entries
  std::vector<Entry> mTable{INITIAL_SIZE, Entry{}};
  /// the number of filled entries
  size_t mSize = 0;
  /// the hash function we use
  std::hash<K> mHasher;

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
        _insertWithHash(std::move(entry.key), std::move(entry.value),
                        entry.hash);
      }
    }
  }

  /**
   * Inserts a key-value pair into the map using a precomputed hash.
   * This is used internally during resizing to avoid recomputing hashes.
   * @param key The key to insert.
   * @param value The value associated with the key.
   * @param hash The precomputed hash of the key.
   */
  void _insertWithHash(K key, V value, size_t hash) {
    const size_t mask = mTable.size() - 1;
    size_t index = hash & mask;
    size_t probeCount = 0;

    while (mTable[index].filled) {
      // If the key already exists update the value and return
      if (mTable[index].hash == hash && mTable[index].key == key) {
        mTable[index].value = std::move(value);
        return;
      }

      // Calculate the probe count of the existing entry
      //
      // same as doing (index - mTable[index].hash % mTable.size()) +
      // mTable.size()) % mTable.size()
      //
      // We use mask instead of the % operator as the mask is bitwise and
      // significantly faster than using the % operator which is a lot more
      // costly, especially since we are trying to optimize for speed
      size_t existingProbeCount =
          (index - (mTable[index].hash & mask) + mTable.size()) & mask;

      // If the existing entry has probed less than the current probe count,
      // swap with it (Robin Hood step)
      if (existingProbeCount < probeCount) {
        std::swap(key, mTable[index].key);
        std::swap(value, mTable[index].value);
        std::swap(hash, mTable[index].hash);
        probeCount = existingProbeCount;
      }

      ++probeCount;
      index = (index + 1) & mask;
    }

    // Insert the new entry into the found slot
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
   * Copy constructor.
   * @param other The other RobinHoodMap to copy from.
   */
  RobinHoodMap(const RobinHoodMap& other)
      : mTable(other.mTable), mSize(other.mSize), mHasher(other.mHasher) {}

  /**
   * Move constructor.
   * @param other The other RobinHoodMap to move from.
   */
  RobinHoodMap(RobinHoodMap&& other) noexcept
      : mTable(std::move(other.mTable)),
        mSize(other.mSize),
        mHasher(std::move(other.mHasher)) {
    other.mSize = 0;
  }

  /**
   * Copy assignment operator.
   * @param other The other RobinHoodMap to copy from.
   * @return Reference to this map.
   */
  RobinHoodMap& operator=(const RobinHoodMap& other) {
    if (this != &other) {
      mTable = other.mTable;
      mSize = other.mSize;
      mHasher = other.mHasher;
    }
    return *this;
  }

  /**
   * Move assignment operator.
   * @param other The other RobinHoodMap to move from.
   * @return Reference to this map.
   */
  RobinHoodMap& operator=(RobinHoodMap&& other) noexcept {
    if (this != &other) {
      mTable = std::move(other.mTable);
      mSize = other.mSize;
      mHasher = std::move(other.mHasher);
      other.mSize = 0;
    }
    return *this;
  }

  /**
   * Inserts a key-value pair into the map.
   * If the key already exists, its value is updated.
   * @param key The key to insert.
   * @param value The value associated with the key.
   */
  void insert(const K& key, const V& value) {
    if (mSize + 1 >= (mTable.size() * RHM_LOAD_FACTOR)) {
      _resize();
    }
    _insertWithHash(key, value, mHasher(key));
  }

  /**
   * Removes a key-value pair from the map given by the key.
   * Will remove the key if it exists, otherwise does nothing.
   * @param key The key to search and remove..
   */
  bool remove(const K& key) {
    const size_t mask = mTable.size() - 1;
    size_t hash = mHasher(key);
    size_t index = hash & mask;
    size_t probeCount = 0;

    while (mTable[index].filled) {
      // Found key
      if (mTable[index].hash == hash && mTable[index].key == key) {
        mTable[index].filled = false;
        --mSize;

        // Now we need to check subsequent entries and shift them back if they
        // are part of the same probe sequence
        size_t nextIndex = (index + 1) & mask;
        while (mTable[nextIndex].filled) {
          // If the next entry is in its home slot, we can stop shifting
          size_t nextProbeCount =
              (nextIndex - (mTable[nextIndex].hash & mask) + mTable.size()) &
              mask;
          if (nextProbeCount == 0) {
            break;
          }

          mTable[index] = std::move(mTable[nextIndex]);
          mTable[nextIndex].filled = false;

          index = nextIndex;
          nextIndex = (nextIndex + 1) & mask;
        }
        return true;
      }

      size_t currentProbeCount =
          (index - (mTable[index].hash & mask) + mTable.size()) & mask;
      if (currentProbeCount < probeCount) {
        return false;
      }

      ++probeCount;
      index = (index + 1) & mask;
    }
    return false;
  }

  /**
   * Retrieves the value associated with the given key.
   * If the key is found, returns a ValueResult with worked=true and a reference
   * to the value. If the key is not found, returns worked=false and a reference
   * to a default value.
   * @param key The key to look up.
   * @return ValueResult containing success flag and reference to value.
   */
  std::expected<V, RHMError> at(const K& key) const {
    const size_t mask = mTable.size() - 1;
    const size_t hash = mHasher(key);
    const size_t homeIndex = hash & mask;
    size_t index = homeIndex;
    size_t probeCount = 0;

    for (;;) {
      const Entry& entry = mTable[index];

      if (!entry.filled) {
        return std::unexpected(RHMError(RHMError::Type::KeyNotFound,
                                        "Provided key not found in map"));
      }

      if (size_t entryProbeCount =
              (index - (entry.hash & mask) + mTable.size()) & mask;
          entryProbeCount < probeCount) {
        return std::unexpected(RHMError(RHMError::Type::KeyNotFound,
                                        "Provided key not found in map"));
      }

      if (entry.hash == hash && entry.key == key) {
        return entry.value;
      }

      ++probeCount;
      index = (index + 1) & mask;
    }
  }

  /**
   * Overloaded const subscript operator to access values by key.
   * Behaves the same as the at() method.
   * @param key The key to look up.
   * @return Expected containing the value if found, or an error message.
   */
  std::expected<V, RHMError> operator[](const K& key) const { return at(key); }

  /**
   * Overloaded non-const subscript operator for insertion/update.
   * If the key does not exist, this will insert it with a default-constructed
   * value
   * @param key The key to look up or insert.
   * @return Reference to the value associated with the key.
   */
  V& operator[](const K& key) {
    const size_t mask = mTable.size() - 1;
    const size_t hash = mHasher(key);
    size_t index = hash & mask;

    size_t searchIndex = index;
    size_t searchProbe = 0;

    for (;;) {
      Entry& entry = mTable[searchIndex];

      if (!entry.filled) {
        break;
      }

      if (size_t entryProbeCount =
              (searchIndex - (entry.hash & mask) + mTable.size()) & mask;
          entryProbeCount < searchProbe) {
        break;
      }

      if (entry.hash == hash && entry.key == key) {
        return entry.value;
      }

      ++searchProbe;
      searchIndex = (searchIndex + 1) & mask;
    }

    // default insert if not found
    insert(key, V{});

    // Recompute mask in case insert caused resize
    const size_t new_mask = mTable.size() - 1;

    // now find the inserted entry to return a reference to its value
    index = hash & new_mask;
    for (;;) {
      Entry& entry = mTable[index];
      if (entry.filled && entry.hash == hash && entry.key == key) {
        return entry.value;
      }
      index = (index + 1) & new_mask;
    }
  }

  /**
   * Checks if the map contains a key.
   * @param key The key to check for.
   * @return true if the key exists, false otherwise.
   */
  [[nodiscard]] bool contains(const K& key) const noexcept {
    const size_t mask = mTable.size() - 1;
    const size_t hash = mHasher(key);
    size_t index = hash & mask;
    size_t probeCount = 0;

    for (;;) {
      const Entry& entry = mTable[index];

      if (!entry.filled) {
        return false;
      }

      if (size_t entryProbeCount =
              (index - (entry.hash & mask) + mTable.size()) & mask;
          entryProbeCount < probeCount) {
        return false;
      }

      if (entry.hash == hash && entry.key == key) {
        return true;
      }

      ++probeCount;
      index = (index + 1) & mask;
    }
  }

  /**
   * Removes all elements from the map.
   */
  void clear() noexcept {
    mTable = std::vector<Entry>(8, Entry{});
    mSize = 0;
  }

  /**
   * Checks if the map is empty.
   * @return true if the map has no elements, false otherwise.
   */
  [[nodiscard]] bool empty() const noexcept { return mSize == 0; }

  /**
   * Returns the number of elements in the map.
   * @return The size of the map.
   */
  [[nodiscard]] size_t size() const noexcept { return mSize; }

  /**
   * Pre-allocates space for at least the specified number of elements.
   * @param capacity The minimum capacity to reserve.
   */
  void reserve(size_t capacity) {
    size_t newSize = 8 << (std::bit_width(capacity << 1));

    // Only resize if the new size is larger than the current table size
    if (newSize > mTable.size()) {
      std::vector<Entry> oldTable = std::move(mTable);
      mTable = std::vector<Entry>(newSize, Entry{});
      mSize = 0;

      for (auto& entry : oldTable) {
        if (entry.filled) {
          _insertWithHash(std::move(entry.key), std::move(entry.value),
                          entry.hash);
        }
      }
    }
  }

  /**
   * Iterator class for RobinHoodMap entries.
   */
  class Iterator {
   private:
    Entry* mEntry;  //< Current entry pointer
    Entry* mEnd;    //< End entry pointer

    friend class RobinHoodMap;

   public:
    /// Constructs an iterator pointing to a specific entry.
    /// @param ptr Pointer to the current entry.
    /// @param end Pointer to the end entry.
    explicit Iterator(Entry* ptr, Entry* end = nullptr)
        : mEntry(ptr), mEnd(end) {}

    /// Dereferences the iterator to access the current entry.
    /// @return Reference to the current entry.
    Entry& operator*() {
      assert(mEntry != mEnd && "Dereferencing end iterator");
      assert(mEntry->filled && "Dereferencing an empty slot");
      return *mEntry;
    }

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
      while (mEntry != mEnd && !mEntry->filled) {
        --mEntry;
      }
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
   * Const Iterator class for RobinHoodMap entries.
   */
  class ConstIterator {
   private:
    const Entry* mEntry;  //< Current entry pointer
    const Entry* mEnd;    //< End entry pointer

    friend class RobinHoodMap;

   public:
    /// Constructs a const iterator pointing to a specific entry.
    /// @param ptr Pointer to the current entry.
    /// @param end Pointer to the end entry.
    explicit ConstIterator(const Entry* ptr, const Entry* end = nullptr)
        : mEntry(ptr), mEnd(end) {}

    /// Dereferences the iterator to access the current entry.
    /// @return Const reference to the current entry.
    const Entry& operator*() const { return *mEntry; }

    /// Pre-increment operator to move to the next filled entry.
    /// @return the updated iterator.
    ConstIterator& operator++() {
      ++mEntry;
      while (mEntry != mEnd && !mEntry->filled) {
        ++mEntry;
      }
      return *this;
    }

    /// Pre-decrement operator to move to the previous entry.
    /// @return the updated iterator.
    ConstIterator& operator--() {
      --mEntry;
      return *this;
    }

    /// Post-increment operator to move to the next filled entry.
    /// @return the original iterator before increment.
    ConstIterator operator++(int) {
      ConstIterator temp = *this;
      ++(*this);
      return temp;
    }

    /// Post-decrement operator to move to the previous entry.
    /// @return the original iterator before decrement.
    ConstIterator operator--(int) {
      ConstIterator temp = *this;
      --(*this);
      return temp;
    }

    /**
     * Inequality operator to compare two iterators.
     * @param other The other iterator to compare with.
     * @return true if the iterators are not equal, false otherwise.
     */
    bool operator!=(const ConstIterator& other) const {
      return mEntry != other.mEntry;
    }

    /**
     * Equality operator to compare two iterators.
     * @param other The other iterator to compare with.
     * @return true if the iterators are equal, false otherwise.
     */
    bool operator==(const ConstIterator& other) const {
      return mEntry == other.mEntry;
    }

    /**
     * Addition operator to advance the iterator by an offset.
     * @param offset The number of positions to advance.
     * @return A new iterator advanced by the specified offset.
     */
    ConstIterator operator+(size_t offset) const {
      return ConstIterator(mEntry + offset, mEnd);
    }

    /**
     * Subtraction operator to move the iterator backward by an offset.
     * @param offset The number of positions to move backward.
     * @return A new iterator moved backward by the specified offset.
     */
    ConstIterator operator-(size_t offset) const {
      return ConstIterator(mEntry - offset, mEnd);
    }
  };

  using const_iterator = ConstIterator;

  /**
   * Returns an iterator to the first filled entry in the map.
   * @return Iterator pointing to the first filled entry.
   */
  Iterator begin() {
    // find the first filled entry
    auto it = std::find_if(mTable.begin(), mTable.end(),
                           [](const auto& entry) { return entry.filled; });

    // if no entries are filled, return end(), otherwise rerturn an iterator
    // to the first entry.
    if (it == mTable.end()) return end();
    return Iterator(&*it, mTable.data() + mTable.size());
  }

  /**
   * Returns an iterator to one past the last entry in the map.
   * @return Iterator pointing to one past the last entry.
   */
  Iterator end() {
    return Iterator(mTable.data() + mTable.size(),
                    mTable.data() + mTable.size());
  }

  /**
   * Returns a const iterator to the first filled entry in the map.
   * @return Const iterator pointing to the first filled entry.
   */
  ConstIterator begin() const {
    auto it = std::find_if(mTable.cbegin(), mTable.cend(),
                           [](const auto& entry) { return entry.filled; });
    if (it == mTable.cend()) return end();
    return ConstIterator(&*it, mTable.data() + mTable.size());
  }

  /**
   * Returns a const iterator to one past the last entry in the map.
   * @return Const iterator pointing to one past the last entry.
   */
  ConstIterator end() const {
    return ConstIterator(mTable.data() + mTable.size(),
                         mTable.data() + mTable.size());
  }

  /**
   * Returns a const iterator to the first filled entry in the map.
   * @return Const iterator pointing to the first filled entry.
   */
  ConstIterator cbegin() const { return begin(); }

  /**
   * Returns a const iterator to one past the last entry in the map.
   * @return Const iterator pointing to one past the last entry.
   */
  ConstIterator cend() const { return end(); }

  friend class ::RobinHoodMapTest;
};
}  // namespace cse498