/**
 * @file DataMap.hpp
 * @brief Declaration of the DataMap class
 * @author Cindy Huang
 * @date 2026-02-05
 */

#pragma once

#include <string>
#include <unordered_map>
#include <any>
#include <cassert>
#include <expected>

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
                 * Dereference operator returns a reference to the underlying std::pair<const std::string, std::any>.
                 */
                reference operator*() const { return *mIt; }
                /**
                 * Arrow operator overload
                 */
                pointer operator->() const { return &(*mIt); }

                /**
                 * Pre-increment and post-increment operators to advance the iterator.
                 */
                Iterator& operator++() { ++mIt; return *this; }
                Iterator operator++(int) { Iterator tmp = *this; ++mIt; return tmp; }

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
                V As() const { return std::any_cast<V>(mIt->second); }

                /// Convenience: get a reference to the stored value of the specified type.
                template <typename V>
                V& AsRef() { return *std::any_cast<V>(&mIt->second); }
        };

        /// Const iterator class that wraps the underlying unordered_map const_iterator.
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
                 * Dereference operator returns a reference to the underlying std::pair<const std::string, std::any>.
                 */
                reference operator*() const { return *mIt; }
                /**
                 * Arrow operator overload
                 */
                pointer operator->() const { return &(*mIt); }

                /**
                 * Pre-increment and post-increment operators to advance the iterator.
                 */
                ConstIterator& operator++() { ++mIt; return *this; }
                ConstIterator operator++(int) { ConstIterator tmp = *this; ++mIt; return tmp; }

                /**
                 * Equality and inequality operators to compare iterators.
                 */
                bool operator==(const ConstIterator& other) const { return mIt == other.mIt; }
                bool operator!=(const ConstIterator& other) const { return mIt != other.mIt; }

                /**
                 * Returns the key of the current element.
                 */
                const std::string& Key() const { return mIt->first; }
                /**
                 * Returns the std::any value of the current element.
                 */
                const std::any& Value() const { return mIt->second; }

                template <typename V>
                V As() const { return std::any_cast<V>(mIt->second); }
        };

        // --- Iterator access ---
        Iterator begin() { return Iterator(mData.begin()); }
        Iterator end() { return Iterator(mData.end()); }
        ConstIterator begin() const { return ConstIterator(mData.cbegin()); }
        ConstIterator end() const { return ConstIterator(mData.cend()); }
        ConstIterator cbegin() const { return ConstIterator(mData.cbegin()); }
        ConstIterator cend() const { return ConstIterator(mData.cend()); }

        template <typename V>
        void Set(const std::string& name, const V& value) { mData[name] = value;}

        template <typename V>
        std::expected<V, std::string> Get(const std::string& name) const
        {
            auto it = mData.find(name);
            if (it == mData.end()) {
                return std::unexpected("Key not found");
            }
            if (it->second.type() != typeid(V)) {
                return std::unexpected("Type mismatch for key");
            }
            return std::any_cast<V>(it->second);
        }

        template <typename V>
        V& GetRef(const std::string& name)
        {
            auto it = mData.find(name);
            assert(it != mData.end() && "Key not found");
            auto* p = std::any_cast<V>(&it->second);
            assert(p != nullptr && "Key returns a nullptr");
            return *p;
        }

        bool Contains(const std::string& name) const { return mData.find(name) != mData.end(); }
        bool IsEmpty() const noexcept { return mData.empty(); }
        int Size() const { return mData.size(); }
        void Clear() { mData.clear(); }
};
}