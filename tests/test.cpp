#include <catch2/catch_test_macros.hpp>
#include <string>
#include <utility>

#include "al/array_list.hpp"

TEST_CASE("Basic functionality") {
  al::ArrayList<int> list;
  REQUIRE(list.empty());
  REQUIRE(list.size() == 0);
  REQUIRE(list.begin() == list.end());
  REQUIRE(list.cbegin() == list.cend());

  list.push_back(42);
  REQUIRE(list.size() == 1);
  REQUIRE(list.front() == 42);
  REQUIRE(list.back() == 42);

  list.pop_back();
  REQUIRE(list.size() == 0);
  REQUIRE(list.empty());
}

TEST_CASE("Iterators") {
  al::ArrayList<int> list;
  REQUIRE(list.begin() == list.end());
  REQUIRE(list.cbegin() == list.cend());

  list.push_back(42);
  REQUIRE(list.begin() != list.end());
  REQUIRE(list.cbegin() != list.cend());

  list.pop_back();
  REQUIRE(list.begin() == list.end());
  REQUIRE(list.cbegin() == list.cend());
}

TEST_CASE("Iterators with non-trivial types") {
  static int x = 0;
  struct Foo {
    Foo() = default;
    Foo& operator=(const Foo&) = delete;
    Foo(const Foo&) = delete;
    Foo(Foo&& other) noexcept { other.counts = false; }
    Foo& operator=(Foo&& other) noexcept {
      other.counts = false;
      return *this;
    }
    ~Foo() { x++; }
    bool counts = true;
  };
  {
    al::ArrayList<Foo> list;

    list.emplace_back();
    list.emplace_back();
    REQUIRE(x == 0);
  }

  REQUIRE(x == 2);
}

TEST_CASE("Copy construction") {
  al::ArrayList<int> list;
  list.push_back(42);

  al::ArrayList<int> copy = list;
  REQUIRE(copy.size() == 1);
  REQUIRE(copy.front() == 42);
}

TEST_CASE("Copy with non trivial types") {
  al::ArrayList<std::string> list;
  list.push_back("Hello");
  list.push_back("World");

  al::ArrayList<std::string> copy = list;
  REQUIRE(copy.size() == 2);
  REQUIRE(copy.front() == "Hello");
  REQUIRE(copy.back() == "World");
}
