#include <cstddef>
#include <iostream>
#include <utility>

#include "array_list.hpp"

template <typename Type>
class LinkedList;

template <typename Type>
struct Node {
  constexpr auto take_ptr(auto*& ptr) -> auto* {
    auto* tmp{ptr};
    ptr = nullptr;
    return tmp;
  }

 public:
  using NodeType = Node<Type>;
  constexpr Node(Type&& data)  // NOLINT
      : data_(std::move(data)) {}
  constexpr Node(const Type& data)  // NOLINT
      : data_(data) {}

  Node(const NodeType&) = delete;
  Node(NodeType&& take_me) noexcept
      : data_(std::move(take_me.data_)), next_(take_ptr(take_me.next_)) {}
  auto operator=(const NodeType&) -> Node& = delete;
  constexpr auto operator=(NodeType&& take_me) noexcept -> Node& {
    data_ = std::move(take_me.data_);
    next_ = take_ptr(take_me.next_);
    return *this;
  }

  ~Node() { delete next_; }

  constexpr operator Type&() noexcept  // NOLINT
  {
    return data_;
  }
  constexpr operator const Type&() const noexcept  // NOLINT
  {
    return data_;
  }

  constexpr auto data() noexcept -> Type& { return data_; }
  constexpr auto data() const noexcept -> const Type& { return data_; }

  auto operator*() noexcept -> Type& { return data(); }
  auto operator*() const noexcept -> const Type& { return data(); }

 private:
  friend LinkedList<Type>;

  Type data_;
  NodeType* next_{nullptr};
};

template <typename Type>
class LinkedList {
 public:
  using NodeType = Node<Type>;

  LinkedList() = default;

  LinkedList(auto&&... args) {  // NOLINT
    (push_back(std::forward<decltype(args)>(args)), ...);
  }

  auto append_front(auto&&... args) -> void {
    auto* new_node{new NodeType(Type{std::forward<decltype(args)>(args)...})};
    new_node->next_ = head_;
    head_ = new_node;
    if (not tail_) {
      tail_ = new_node;
    }
  }

  auto push_back(auto&&... args) -> void {
    auto* new_node{new NodeType(Type{std::forward<decltype(args)>(args)...})};
    _add_to_end(new_node);
  }

  ~LinkedList() { delete head_; }

  struct iterator {  // NOLINT
   public:
    constexpr inline iterator(NodeType* data)  // NOLINT
        : data_(data) {}
    constexpr inline iterator(std::nullptr_t)  // NOLINT
        : data_(nullptr) {}

    constexpr auto operator!=(const iterator& other) const noexcept -> bool {
      return data_ != other.data_;
    }
    constexpr auto operator==(const iterator& other) const noexcept -> bool {
      return data_ == other.data_;
    }

    auto operator++() noexcept -> iterator& {
      if (data_) {
        data_ = data_->next_;
      }
      return *this;
    }

    auto operator*() noexcept -> Type& { return data_->data(); }
    auto operator*() const noexcept -> const Type& { return data_->data(); }

   private:
    NodeType* data_;
  };

  auto begin() noexcept -> iterator { return head_; }
  auto end() noexcept -> iterator { return nullptr; }
  auto begin() const noexcept -> iterator { return head_; }
  auto end() const noexcept -> iterator { return nullptr; }

 private:
  // NOLINTBEGIN
  constexpr auto _add_to_end(NodeType* new_node) {
    if (not head_) {
      head_ = new_node;
    }

    if (tail_) {
      tail_->next_ = new_node;
    }
    tail_ = new_node;
  }
  // NOLINTEND

  NodeType* head_{nullptr};
  NodeType* tail_{nullptr};
};

struct Foo {
  ~Foo() { std::cerr << "Deleting Foo\n"; }
};
auto operator<<(std::ostream& os,
                [[maybe_unused]] const Foo& foo) -> std::ostream& {
  return os << "Foo!";
}

auto main() -> int {
  ghj::ArrayList<int> n{1, 2, 4, 7, 8, 12};
  for (const auto& num : n) {
    std::cerr << num << '\n';
  }

  LinkedList<Foo> list{};
  list.push_back();
  list.push_back();

  for (auto& node : list) {
    std::cout << node << '\n';
  }
}
