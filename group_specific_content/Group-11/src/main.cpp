#include "FunctionSet.hpp"
#include <print>

void dd(int y) {
    std::println("{}", y);
}

int main() {
    FunctionSet<void, int> x{dd};


    x.add([](int x) { std::println("{}",x); });


    for (auto func: x) {
        func(5);
    }

}