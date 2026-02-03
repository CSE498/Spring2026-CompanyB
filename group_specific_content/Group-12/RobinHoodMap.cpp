/**
 * @file RobinHoodMap.cpp
 * @brief Implementation of the RobinHoodMap class.
 * @author John Stouffer
 * @date 2026-2-2
 */

#include "RobinHoodMap.hpp"

/**
 * @brief Inserts a key-value pair into the our hash map.
 */
template<typename K, typename V>
void RobinHoodMap<K, V>::insert(const K& key, const V& value) 
{
    return;
}

/**
 * @brief Removes a key-value pair from our hash map by key.
 */
template<typename K, typename V>
void RobinHoodMap<K, V>::remove(const K& key) 
{
    return;
}

/**
 * @brief Accesses the value associated with the given key.
 * 
 * @throws std::out_of_range if the key is not found.
 */
template<typename K, typename V>
typename RobinHoodMap<K, V>::ValueResult RobinHoodMap<K, V>::at(const K& key) const
{
    return mTable[0].value;
};

template<typename K, typename V>
typename RobinHoodMap<K, V>::ValueResult RobinHoodMap<K, V>::operator[](const K& key) const
{
    return mTable[0].value;
};

template<typename K, typename V>
void RobinHoodMap<K, V>::operator=(const RobinHoodMap<K, V>& other)
{
    mTable = other.mTable;
    mSize = other.mSize;
}

template<typename K, typename V>
size_t RobinHoodMap<K, V>::size() const
{
    return mSize;
}

