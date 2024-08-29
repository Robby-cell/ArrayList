#ifndef ARRAY_LIST_HPP
#define ARRAY_LIST_HPP

#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace al {

consteval inline auto operator""_UZ(const unsigned long long value) -> size_t {
  return static_cast<size_t>(value);
}

namespace detail {

template <class Iter>
using IterCatType = typename std::iterator_traits<Iter>::iterator_category;

template <class Type, class = void>
constexpr bool IsIterator = false;

template <class Type>
constexpr bool IsIterator<Type, std::void_t<IterCatType<Type>>> = true;

template <class Iter, class = void>
constexpr bool AllowInheritingUnwrap = true;

template <class Iter>
constexpr bool AllowInheritingUnwrap<
    Iter, std::void_t<typename Iter::_Prevent_inheriting_unwrap>> =
    std::is_same_v<Iter, typename Iter::_Prevent_inheriting_unwrap>;

template <class Iter, class = void>
constexpr bool IsUnwrappable = false;

template <class Iter>
constexpr bool IsUnwrappable<
    Iter,
    std::void_t<decltype(std::declval<std::remove_cvref_t<Iter>&>()._Seek_to(
        std::declval<Iter>()._Unwrapped()))>> =
    AllowInheritingUnwrap<std::remove_cvref_t<Iter>>;

template <class Iter, class = void>
constexpr bool HasNothrowUnwrapped = false;
template <class Iter>
constexpr bool HasNothrowUnwrapped<
    Iter, std::void_t<decltype(std::declval<Iter>()._Unwrapped())>> =
    noexcept(std::declval<Iter>()._Unwrapped());

template <class Iter>
constexpr auto get_unwrapped(Iter&& it) noexcept(
    !IsUnwrappable<Iter> || HasNothrowUnwrapped<Iter>) -> decltype(auto) {
  if constexpr (std::is_pointer_v<std::decay_t<Iter>>) {
    return it + 0;
  } else if constexpr (IsUnwrappable<Iter>) {
    return static_cast<Iter&&>(it)._Unwrapped();
  } else {
    return static_cast<Iter&&>(it);
  }
}
}  // namespace detail

template <typename Type, typename Allocator = std::allocator<Type>>
  requires(std::is_same_v<Type, std::remove_reference_t<Type>>)
class ArrayList {
  static constexpr inline auto GrowthFactor = 2_UZ;

 public:
  // NOLINTBEGIN
  using allocator = Allocator;
  using allocator_traits = std::allocator_traits<allocator>;
  using value_type = std::remove_const_t<Type>;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  // NOLINTEND

 private:
  struct Iterator {
   public:
    // NOLINTBEGIN
    using iterator_category = std::random_access_iterator_tag;
    using value_type = value_type;
    using difference_type = difference_type;
    using pointer = pointer;
    using reference = reference;

    constexpr Iterator(pointer current) : current_(current) {}
    constexpr Iterator(std::nullptr_t) : current_(nullptr) {}
    // NOLINTEND

    constexpr auto operator*() noexcept -> reference { return *current_; }
    constexpr auto operator*() const noexcept -> const_reference {
      return *current_;
    }
    constexpr auto operator->() noexcept -> pointer { return current_; }
    constexpr auto operator->() const noexcept -> const_pointer {
      return current_;
    }

    constexpr auto operator++() noexcept -> Iterator& {
      ++current_;
      return *this;
    }
    constexpr auto operator++(int) noexcept -> Iterator {
      auto tmp = *this;
      ++current_;
      return tmp;
    }

    friend constexpr auto operator!=(const Iterator& me,
                                     const Iterator& other) noexcept {
      return me.current_ != other.current_;
    }
    friend constexpr auto operator==(const Iterator& me,
                                     const Iterator& other) noexcept {
      return me.current_ == other.current_;
    }

   private:
    pointer current_;
  };
  struct ConstIterator {
   public:
    // NOLINTBEGIN
    using iterator_category = std::random_access_iterator_tag;
    using value_type = value_type;
    using difference_type = difference_type;
    using pointer = const_pointer;
    using reference = const_reference;

    constexpr ConstIterator(pointer current) : current_(current) {}
    constexpr ConstIterator(std::nullptr_t) : current_(nullptr) {}
    // NOLINTEND

    constexpr auto operator*() noexcept -> const_reference { return *current_; }
    constexpr auto operator*() const noexcept -> const_reference {
      return *current_;
    }
    constexpr auto operator->() noexcept -> const_pointer { return current_; }
    constexpr auto operator->() const noexcept -> const_pointer {
      return current_;
    }

    constexpr auto operator++() noexcept -> Iterator& {
      ++current_;
      return *this;
    }
    constexpr auto operator++(int) noexcept -> Iterator {
      auto tmp = *this;
      ++current_;
      return tmp;
    }

    friend constexpr auto operator!=(const ConstIterator& me,
                                     const ConstIterator& other) noexcept {
      return me.current_ != other.current_;
    }
    friend constexpr auto operator==(const ConstIterator& me,
                                     const ConstIterator& other) noexcept {
      return me.current_ == other.current_;
    }

   private:
    const_pointer current_;
  };

 public:
  // NOLINTBEGIN
  using iterator = Iterator;
  using const_iterator = ConstIterator;
  // NOLINTEND

  constexpr explicit ArrayList(const size_type capacity = 8_UZ)
      : data_(allocator_traits::allocate(my_allocator, capacity)) {
    end_ = data_ + capacity;
    current_ = data_;
  }
  template <typename Iter, std::enable_if_t<detail::IsIterator<Iter>, int> = 0>
  constexpr ArrayList(Iter first, Iter last,
                      [[maybe_unused]] const allocator& alloc = allocator()) {
    auto ufirst = detail::get_unwrapped(first);
    auto ulast = detail::get_unwrapped(last);

    const auto length = static_cast<size_t>(std::distance(ufirst, ulast));

    data_ = allocator_traits::allocate(my_allocator, length);
    end_ = data_ + length;
    current_ = data_ + length;

    std::uninitialized_copy(ufirst, ulast, data_);
  }

  constexpr ArrayList(std::initializer_list<Type> list,
                      [[maybe_unused]] const allocator& alloc = allocator())
      : ArrayList(list.begin(), list.end()) {}

  constexpr ArrayList(const ArrayList& other,
                      [[maybe_unused]] const allocator& alloc = allocator())
      : ArrayList(other.begin(), other.end(), alloc) {}
  constexpr ArrayList(ArrayList&& other) noexcept
      : data_(other.data_), end_(other.end_), current_(other.current_) {
    other.data_ = nullptr;
    other.end_ = nullptr;
    other.current_ = nullptr;
  }

  constexpr auto operator=(const ArrayList& other) -> ArrayList& {
    if (this != &other) {
      copy_safe(other);
    }
    return *this;
  }
  constexpr auto operator=(ArrayList&& other) noexcept -> ArrayList& {
    destruct_all_elements();
    deallocate_ptr();

    data_ = other.data_;
    end_ = other.end_;
    current_ = other.current_;

    other.data_ = nullptr;
    other.end_ = nullptr;
    other.current_ = nullptr;

    return *this;
  }

  ~ArrayList() {
    destruct_all_elements();
    deallocate_ptr();
  }

  constexpr inline auto empty() const noexcept -> bool { return size() == 0; }

  void push_back(const Type& value) {
    ensure_size_for_elements(1_UZ);
    raw_push_back(value);
  }
  void push_back(Type&& value) {
    ensure_size_for_elements(1_UZ);
    raw_push_back(std::move(value));
  }
  template <typename... Args>
  void emplace_back(Args&&... args) {
    ensure_size_for_elements(1_UZ);
    raw_emplace_back(std::forward<Args>(args)...);
  }

  constexpr void pop_back() {
    ensure_not_empty();
    --current_;
  }

  [[nodiscard]] constexpr auto size() const noexcept -> size_type {
    return current_ - data_;
  }

  void set_capacity(const size_type new_capacity) {
    const auto cap = capacity();
    const auto len = size();
    const auto old_ptr = data_;
    raw_set_capacity(new_capacity);
    if (old_ptr) {
      if (len > 0) {
        std::uninitialized_copy(old_ptr, old_ptr + len, data_);
      }
      if (cap > 0) {
        allocator_traits::deallocate(my_allocator, old_ptr, cap);
      }
    }
  }

  [[nodiscard]] constexpr auto operator[](const size_type index) noexcept
      -> reference {
    return data_[index];
  }
  [[nodiscard]] constexpr auto operator[](const size_type index) const noexcept
      -> const_reference {
    return data_[index];
  }
  [[nodiscard]] constexpr auto at(const size_type index) -> reference {
    ensure_in_range(index);
    return data_[index];
  }
  [[nodiscard]] constexpr auto at(const size_type index) const
      -> const_reference {
    ensure_in_range(index);
    return data_[index];
  }

  [[nodiscard]] constexpr auto capacity() const noexcept -> size_type {
    return end_ - data_;
  }

  [[nodiscard]] constexpr auto data() noexcept -> pointer { return data_; }
  [[nodiscard]] constexpr auto data() const noexcept -> const_pointer {
    return data_;
  }

  [[nodiscard]] constexpr auto front() -> reference {
    ensure_not_empty();
    return *data_;
  }
  [[nodiscard]] constexpr auto front() const -> const_reference {
    ensure_not_empty();
    return *data_;
  }

  [[nodiscard]] constexpr auto back() -> reference {
    ensure_not_empty();
    return *(current_ - 1);
  }
  [[nodiscard]] constexpr auto back() const -> const_reference {
    ensure_not_empty();
    return *(current_ - 1);
  }

  [[nodiscard]] constexpr auto begin() noexcept -> iterator { return data_; }
  [[nodiscard]] constexpr auto end() noexcept -> iterator { return current_; }
  [[nodiscard]] constexpr auto cbegin() noexcept -> iterator { return data_; }
  [[nodiscard]] constexpr auto cend() noexcept -> iterator { return current_; }
  [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator {
    return data_;
  }
  [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
    return current_;
  }
  [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator {
    return data_;
  }
  [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator {
    return current_;
  }

  constexpr auto clear() noexcept -> void {
    destruct_all_elements();
    current_ = data_;
  }

 private:
  constexpr void raw_set_capacity(const size_type capacity) {
    const auto length = size();
    data_ = allocator_traits::allocate(my_allocator, capacity);
    current_ = data_ + length;
    end_ = data_ + capacity;
  }

  constexpr void copy_safe(const ArrayList& other) {
    destruct_all_elements();
    deallocate_ptr();
    copy_unsafe(other);
  }

  constexpr void copy_unsafe(const ArrayList& other) {
    const auto length = other.size();
    set_capacity(length);
    current_ = data_ + length;
    for (auto i = 0_UZ; i < length; ++i) {
      data_[i] = other.data_[i];
    }
  }

  constexpr void ensure_in_range(const size_type index) const {
    if (index >= size() or data_ == nullptr) {
      throw std::out_of_range("Index out of range");
    }
  }

  constexpr void ensure_not_empty() const {
    if (size() <= 0_UZ or data_ == nullptr) {
      throw std::out_of_range("ArrayList is empty");
    }
  }

  void ensure_size_for_elements(const size_type elements) {
    while ((current_ + elements) > end_) {
      grow_capacity();
    }
  }
  void grow_capacity() { set_capacity((end_ - data_) * GrowthFactor); }

  template <typename... Args>
  constexpr void raw_emplace_back(Args&&... args) {
    raw_emplace_into(current_++, std::forward<Args>(args)...);
  }
  constexpr void raw_push_back(const Type& value) {
    raw_push_into(current_++, value);
  }
  constexpr void raw_push_back(Type&& value) {
    raw_push_into(current_++, std::move(value));
  }

  template <typename... Args>
  constexpr void raw_emplace_into(value_type* const my_ptr, Args&&... args) {
    *my_ptr = Type(std::forward<Args>(args)...);
  }
  constexpr void raw_push_into(value_type* const my_ptr, const Type& value) {
    *my_ptr = value;
  }
  constexpr void raw_push_into(value_type* const my_ptr, Type&& value) {
    *my_ptr = std::move(value);
  }

  constexpr void destruct_all_elements() {
    if constexpr (not std::is_trivially_destructible_v<Type>) {
      if (data_) {
        for (auto& item : *this) {
          item.~Type();
        }
      }
    }
  }
  constexpr void deallocate_ptr() {
    if (data_) {
      allocator_traits::deallocate(my_allocator, data_, end_ - data_);
    }
  }

  pointer data_;
  pointer end_;
  pointer current_;

  static inline allocator my_allocator;
};

}  // namespace al

#endif  // ARRAY_LIST_HPP
