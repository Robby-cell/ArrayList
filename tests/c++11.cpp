#include <catch2/catch_test_macros.hpp>

// Std
#include <algorithm>
#include <iterator>

#include "al/array_list.hpp"

namespace cpp11_test {

void do_simple_test() {
    static constexpr int Count = 10;
    al::ArrayList<int> numbers;
    std::generate_n(std::back_inserter(numbers), Count,
                    [n = 0]() mutable { return n++; });

    for (int i = 0; i < Count; ++i) {
        const auto index = static_cast<std::size_t>(i);
        REQUIRE(numbers.at(index) == i);
    }
}

}  // namespace cpp11_test
