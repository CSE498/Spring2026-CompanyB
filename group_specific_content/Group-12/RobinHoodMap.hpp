/**
 * @file RobinHoodMap.hpp
 * @brief Declaration of the RobinHoodMap class.
 * @author John Stouffer
 * @date 2026-2-2
 */

#pragma once

#include <vector>

/**
 * @brief A hash map implementation using the Robin Hood hashing algorithm 
 * to create a more efficient hash table data structure than the standard
 * library's std::unordered_map.
 */
template<typename K, typename V>
class RobinHoodMap {
private:

    std::vector<Entry> mTable; ///< The array used for the table
    size_t mSize;              ///< The number of entries in the table

    /**
     * @brief Entry struct to hold the key-value pairs within the table.
     */
    struct Entry {
        K key;                 ///< The key of the entry
        V value;               ///< The value of the entry
        bool filled = false;   ///< Flag to indicate if the entry is filled
    };

    /**
     * @brief Result struct for key lookups.
     */
    struct KeyResult {
        bool worked;          ///< Indicates if the lookup was successful
        K& key;               ///< Reference to the found key
    };

    /**
     * @brief Result struct for value lookups.
     */
    struct ValueResult {
        bool worked;          ///< Indicates if the lookup was successful
        V& value;             ///< Reference to the found value
    };

    /**
     * @brief Finds the first filled element in the table.
     * @return A KeyResult with success flag and key reference.
     */
    KeyResult _findFirstElement() {
        for (size_t i = 0; i < mSize; ++i) {
            if (mTable[i].filled) {
                return {true, mTable[i].key};
            }
        }
        return {false, mTable[0].key};
    }

    void _resize() {
        // TODO: Implement resizing logic
    }

public:
    /// @brief Default Constructor
    RobinHoodMap() = default;

    /// @brief Default Destructor
    ~RobinHoodMap() = default;

    void insert(const K& key, const V& value);
    void remove(const K& key);
    ValueResult at(const K& key) const;
    ValueResult operator[](const K& key) const;
    size_t size() const;
    void operator=(const RobinHoodMap<K, V>& other);

    /**
     * @brief Iterator class for RobinHoodMap
     */
    class Iterator {
    private:
        /// @brief Pointer to the current entry in the table
        Entry* mEntry;

    public:
        explicit Iterator(Entry* ptr) : mEntry(ptr) {}

        Entry& operator*() { return *mEntry; }

        Iterator& operator++() {
            ++mEntry;
            return *this;
        }

        Iterator& operator--() {
            --mEntry;
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            ++mEntry;
            return temp;
        }

        Iterator operator--(int) {
            Iterator temp = *this;
            --mEntry;
            return temp;
        }

        bool operator!=(const Iterator& other) const {
            return mEntry != other.mEntry;
        }

        bool operator==(const Iterator& other) const {
            return mEntry == other.mEntry;
        }

        Iterator operator+(size_t offset) const {
            return Iterator(mEntry + offset);
        }

        Iterator operator-(size_t offset) const {
            return Iterator(mEntry - offset);
        }

        Iterator begin() 
        { 
            auto [worked, result] = _findFirstElement();
            if (worked) {
                return Iterator(&result);
            } else {
                return Iterator(&mTable[mSize]);
            }
        }

        Iterator end() { return Iterator(&mTable[mSize]); }
    };
};
