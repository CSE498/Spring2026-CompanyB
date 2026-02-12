#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <ranges>
#include <vector>

template <typename Ret, typename... Params>
class FunctionSet {
    using FuncType = std::function<Ret(Params...)>;
    
    std::vector<FuncType> funcs;

public:
    FunctionSet() = default;

    FunctionSet(std::initializer_list<FuncType> list) {
        for (const auto& val : list) {
            add(val);
        }
    }

    constexpr void add(const FuncType& func) {
        funcs.emplace_back(func);
    }

    // Calling the set
    constexpr void invoke(Params... args) const {
        std::ranges::for_each(funcs, [&](const FuncType& func){
            func(args...);
        });
    }
    
    constexpr void operator()(Params... args) const {
        invoke(args...);
    }

    // Accessing stored functions
    [[nodiscard]] constexpr FuncType& at(size_t index) {
        return funcs.at(index);
    }
    
    [[nodiscard]] constexpr const FuncType& at(size_t index) const {
        return funcs.at(index);
    }

    [[nodiscard]] constexpr FuncType& operator[](size_t index) noexcept {
        return funcs[index];
    }
    
    [[nodiscard]] constexpr const FuncType& operator[](size_t index) const noexcept {
        return funcs[index];
    }

    // Querying certain properties
    constexpr void clear() noexcept {
        funcs.clear();
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return funcs.empty();
    }

    [[nodiscard]] constexpr size_t size() const noexcept {
        return funcs.size();
    }

    // Below this point are stuff for iterators

    [[nodiscard]] constexpr FuncType& front() noexcept {
        return funcs.front();
    }
    [[nodiscard]] constexpr const FuncType& front() const noexcept {
        return funcs.front();
    }

    [[nodiscard]] constexpr FuncType& back() noexcept {
        return funcs.back();
    }
    [[nodiscard]] constexpr const FuncType& back() const noexcept {
        return funcs.back();
    }

    [[nodiscard]] constexpr auto begin() noexcept {
        return funcs.begin();
    }
    [[nodiscard]] constexpr auto begin() const noexcept {
        return funcs.begin();
    }

    [[nodiscard]] constexpr auto end() noexcept {
        return funcs.end();
    }
    [[nodiscard]] constexpr auto end() const noexcept {
        return funcs.end();
    }

    [[nodiscard]] constexpr auto cbegin() const noexcept {
        return funcs.cbegin();
    }
    [[nodiscard]] constexpr auto cend() const noexcept {
        return funcs.cend();
    }

    [[nodiscard]] constexpr auto rbegin() noexcept {
        return funcs.rbegin();
    }
    [[nodiscard]] constexpr auto rbegin() const noexcept {
        return funcs.rbegin();
    }

    [[nodiscard]] constexpr auto rend() noexcept {
        return funcs.rend();
    }
    [[nodiscard]] constexpr auto rend() const noexcept {
        return funcs.rend();
    }

    [[nodiscard]] constexpr auto crbegin() const noexcept {
        return funcs.crbegin();
    }
    [[nodiscard]] constexpr auto crend() const noexcept {
        return funcs.crend();
    }

};