#include <array>
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
    REQUIRE(x == 0);
  }

  SECTION(
      "Make sure no items are destroyed that haven't been constructed and "
      "items that are constructed are destroyed") {
    REQUIRE(x == 2);
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

    REQUIRE(list.size() == Capacity);
    REQUIRE(list.capacity() == Capacity);
  }

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

TEST_CASE("Simple test vs std::vector") {
  constexpr auto WhatToDo =
      []<template <typename, typename> typename ContainerType>() {
        ContainerType<int, std::allocator<int>> list;
        list.reserve(10);
        list.resize(20);

        for (auto i = 0U; i < 20U; ++i) {
          list.emplace_back();
        }

        REQUIRE(list.capacity() == 45);
        REQUIRE(list.size() == 40);
      };

  SECTION("std::vector") { WhatToDo.template operator()<std::vector>(); }
  SECTION("al::ArrayList") { WhatToDo.template operator()<al::ArrayList>(); }
}
