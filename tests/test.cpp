#include <catch2/catch_test_macros.hpp>

#include "al/array_list.hpp"

TEST_CASE("ArrayList") {
  al::ArrayList<int> list;
  REQUIRE(list.empty());
  REQUIRE(list.size() == 0);
  REQUIRE(list.begin() == list.end());
  REQUIRE(list.cbegin() == list.cend());

  list.push_back(42);
  REQUIRE(list.size() == 1);
  REQUIRE(list.front() == 42);
  REQUIRE(list.back() == 42);
}