/**
 * @file Datamap.hpp
 * @brief Declaration of the DataMap class
 * @author Cindy Huang
 * @date 2026-02-05
 */

#pragma once

#include <string>
#include <unordered_map>
#include <any>
#include <cassert>

class DataMap {
    private:
        std::unordered_map<std::string, std::any> mData;

    public:
        template <typename V>
        void Set(const std::string& name, const V& value)
        {
            mData[name] = value;
        }

        template <typename V>
        V Get(const std::string& name)
        { 
            auto it = mData.find(name);
            assert(it != mData.end());
            assert(it->second.type() == typeid(V));
            return std::any_cast<V>(it->second);
        }
        
};