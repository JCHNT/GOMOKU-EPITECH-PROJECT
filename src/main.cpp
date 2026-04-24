#include "protocol/dispatcher.hpp"
#include <iostream>

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    gomoku::Dispatcher d(std::cin, std::cout);
    d.run();
    return 0;
}
