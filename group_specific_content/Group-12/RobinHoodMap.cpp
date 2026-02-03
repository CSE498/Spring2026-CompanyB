/**
 * @file RobinHoodMap.cpp
 * @brief Implementation of the RobinHoodMap class.
 * @author John Stouffer
 * @date 2026-2-2
 */

#include "RobinHoodMap.hpp"

/**
 * @brief Inserts a key-value pair into the map.
 * If the key already exists, its value is updated.
 * @param key The key to insert.
 * @param value The value associated with the key.
 */
template <typename K, typename V>
void RobinHoodMap<K, V>::insert(const K& key, const V& value) {
  if (mSize >= mTable.size() / 2) {
    _resize();
  }
  _insertWithHash(key, value, mHasher(key));
}

template <typename K, typename V>
void RobinHoodMap<K, V>::remove(const K& key) {
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
 * @brief Retrieves the value associated with the given key.
 * If the key is found, returns a ValueResult with worked=true and a reference to the value.
 * If the key is not found, returns worked=false and a reference to a default value.
 * @param key The key to look up.
 * @return ValueResult containing success flag and reference to value.
 */
template <typename K, typename V>
typename RobinHoodMap<K, V>::ValueResult RobinHoodMap<K, V>::at(const K& key) const {
  const size_t mask = mTable.size() - 1;
  size_t hash = mHasher(key);
  size_t index = hash & mask;
  size_t probeCount = 0;

  for (;;) {
    if (!mTable[index].filled) {
      return {false, const_cast<V&>(mTable[0].value)};
    }

    if (mTable[index].hash == hash && mTable[index].key == key) {
      return {true, const_cast<V&>(mTable[index].value)};
    }

    size_t currentProbeCount = (index - (mTable[index].hash & mask) + mTable.size()) & mask;

    if (currentProbeCount < probeCount) {
      return {false, const_cast<V&>(mTable[0].value)};
    }

    ++probeCount;
    index = (index + 1) & mask;
  }
}

/**
 * @brief Overloaded subscript operator to access values by key.
 * Behaves the same as the at() method.
 * @param key The key to look up.
 * @return ValueResult containing success flag and reference to value.
 */
template <typename K, typename V>
typename RobinHoodMap<K, V>::ValueResult RobinHoodMap<K, V>::operator[](const K& key) const {
  return at(key);
}

/**
 * @brief Assignment operator to copy contents from another RobinHoodMap.
 * @param other The other RobinHoodMap to copy from.
 */
template <typename K, typename V>
void RobinHoodMap<K, V>::operator=(const RobinHoodMap<K, V>& other) {
  mTable = other.mTable;
  mSize = other.mSize;
}

/**
 * @brief Returns the number of elements in the map.
 * @return The size of the map.
 */
template <typename K, typename V>
size_t RobinHoodMap<K, V>::size() const {
  return mSize;
}