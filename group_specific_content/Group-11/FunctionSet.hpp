#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <vector>
#include <ranges>

template <typename Ret, typename... Params>
class FunctionSet {
    using FuncType = std::function<Ret(Params...)>;
    
    std::vector<FuncType> funcs;

public:
    FunctionSet() = default;

    constexpr void Add(const FuncType& func) {
        funcs.emplace_back(func);
    }

    // Calling the set
    constexpr void Invoke(Params... args) const {
        std::ranges::for_each(funcs, [&](const FuncType& func){
            func(args...);
        });
    }
    
    constexpr void operator()(Params... args) const {
        Invoke(args...);
    }

    // Accessing stored functions
    [[nodiscard]] constexpr FuncType& At(size_t index) {
        return funcs.at(index);
    }
    
    [[nodiscard]] constexpr const FuncType& At(size_t index) const {
        return funcs.at(index);
    }

    [[nodiscard]] constexpr FuncType& operator[](size_t index) noexcept {
        return funcs[index];
    }
    
    [[nodiscard]] constexpr const FuncType& operator[](size_t index) const noexcept {
        return funcs[index];
    }

    // Querying certain properties
    constexpr void Clear() noexcept {
        funcs.clear();
    }

    [[nodiscard]] constexpr bool Empty() const noexcept {
        return funcs.empty();
    }

    [[nodiscard]] constexpr size_t Size() const noexcept {
        return funcs.size();
    }
};