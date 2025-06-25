#include <array>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <vector>

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
    auto operator=(const Foo&) -> Foo& = delete;
    Foo(const Foo&) = delete;
    Foo(Foo&& other) noexcept { other.counts = false; }
    auto operator=(Foo&& other) noexcept -> Foo& {
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
    REQUIRE(x == 1);
  }

  SECTION(
      "Make sure no items are destroyed that haven't been constructed and "
      "items that are constructed are destroyed") {
    REQUIRE(x == 3);
  }
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

TEST_CASE("Iterator push_back") {
  al::ArrayList<int> list;
  std::array<int, 10> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  list.push_back(values.begin(), values.end());

  REQUIRE(list.size() == 10);
  REQUIRE(list.front() == 1);
  REQUIRE(list.back() == 10);
}

TEST_CASE("Reserve") {
  static int x = 0;
  struct Foo {
    ~Foo() { x++; }
  };
  {
    constexpr size_t Capacity = 20U;

    al::ArrayList<Foo> list;
    list.reserve(Capacity);

    REQUIRE(list.size() == 0);
    REQUIRE(list.capacity() == Capacity);
  }

  SECTION("Make sure no items are destroyed that haven't been constructed") {
    REQUIRE(x == 0);
  }
}

TEST_CASE("Non trivial types in iterator push_back") {
  static int x = 0;
  struct Foo {
    ~Foo() { x++; }
  };

  constexpr size_t Capacity = 20U;
  {
    std::array<Foo, Capacity> values;

    al::ArrayList<Foo> list(Capacity);

    REQUIRE(list.size() == 0);
    REQUIRE(list.capacity() == Capacity);

    list.push_back(values.begin(), values.end());

    REQUIRE(list.size() == values.size());
    REQUIRE(list.capacity() == Capacity);
  }
  REQUIRE(x == Capacity * 2);

  SECTION(
      "Make sure items are destroyed, and their copys are destroyed, and no "
      "more") {
    REQUIRE(x == Capacity * 2);
  }
}

TEST_CASE("Resizing correctly destroys items") {
  static int x = 0;
  struct Foo {
    ~Foo() { x++; }
  };

  constexpr size_t Capacity = 20U;
  {
    al::ArrayList<Foo> list(Capacity);

    list.push_back(Foo{});
    list.push_back(Foo{});

    // copying temporary, twice, 2 calls to destructor
    REQUIRE(x == 2);

    constexpr size_t Size = 0U;
    list.resize(Size);

    REQUIRE(list.capacity() == Capacity);
    REQUIRE(list.size() == Size);
    REQUIRE(x == 4);

    constexpr size_t NewSize = 10U;
    list.reserve(NewSize);

    SECTION("We already have enough space, so need to reserve") {
      REQUIRE(list.capacity() == Capacity);
      REQUIRE(list.size() == Size);
    }
  }
  REQUIRE(x == 4);
}

TEST_CASE("Simple test") {
  constexpr auto WhatToDo = []() {
    al::ArrayList<int> list;
    list.reserve(10);
    list.resize(20);

    for (auto i = 0U; i < 20U; ++i) {
      list.emplace_back();
    }

    REQUIRE(list.capacity() == 45);
    REQUIRE(list.size() == 40);
  };

  SECTION("al::ArrayList") { WhatToDo(); }
}

TEST_CASE("Erase") {
  al::ArrayList<int> list;
  list.push_back(1);
  list.push_back(2);
  list.push_back(3);
  list.push_back(4);
  list.push_back(5);

  REQUIRE(list.size() == 5);
  REQUIRE(list[0] == 1);
  REQUIRE(list[1] == 2);
  REQUIRE(list[2] == 3);
  REQUIRE(list[3] == 4);
  REQUIRE(list[4] == 5);

  auto it = list.erase(const_cast<const al::ArrayList<int>&>(list).begin() + 2);

  REQUIRE(list.size() == 4);
  REQUIRE(list[0] == 1);
  REQUIRE(list[1] == 2);
  REQUIRE(list[2] == 4);
  REQUIRE(list[3] == 5);
}

TEST_CASE("Container-like constructors") {
  std::array<int, 10> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  al::ArrayList<int> list{values};

  REQUIRE(list.size() == 10);
  REQUIRE(list.front() == values.front());
  REQUIRE(list.back() == values.back());
}

TEST_CASE("Construction from std::vector") {
  std::vector<int> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  al::ArrayList<int> list(values);

  for (auto index = 0; index < list.size(); ++index) {
    REQUIRE(list[index] == values[index]);
  }
}

TEST_CASE("Benchmark simple behavior") {
  constexpr auto WhatToDo =
      []<template <typename Type, typename = std::allocator<Type>>
         typename ContainerType>() {
        ContainerType<int> list;
        list.reserve(10);
        list.resize(20);

        for (auto i = 0U; i < 20U; ++i) {
          list.emplace_back();
        }
      };

  BENCHMARK("std::vector") { WhatToDo.template operator()<std::vector>(); };
  BENCHMARK("al::ArrayList") { WhatToDo.template operator()<al::ArrayList>(); };
}
