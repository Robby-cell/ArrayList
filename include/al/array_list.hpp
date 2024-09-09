#ifndef ARRAY_LIST_HPP
#define ARRAY_LIST_HPP

#include <vcruntime_typeinfo.h>

#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace al {

#ifdef AL_NODISCARD
#undef AL_NODISCARD
#endif
#define AL_NODISCARD [[nodiscard]]

template <typename Container>
concept IterableContainer = requires(Container c) {
  c.begin();
  c.end();
};

consteval auto operator""_UZ(const unsigned long long value) -> size_t {
  return static_cast<size_t>(value);
}

namespace detail {

template <class Iter>
using IterConcatenateType =
    typename std::iterator_traits<Iter>::iterator_category;

template <class Type, class = void>
constexpr bool IsIterator = false;

template <class Type>
constexpr bool IsIterator<Type, std::void_t<IterConcatenateType<Type>>> = true;

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
    not IsUnwrappable<Iter> or HasNothrowUnwrapped<Iter>) -> decltype(auto) {
  if constexpr (std::is_pointer_v<std::decay_t<Iter>>) {
    return it + 0;
  } else if constexpr (IsUnwrappable<Iter>) {
    return static_cast<Iter&&>(it)._Unwrapped();
  } else {
    return static_cast<Iter&&>(it);
  }
}

template <typename Iter>
struct IterType {
  using typename Iter::value_type;
};
template <typename Iter>
struct IterType<Iter*> {
  using value_type = Iter;  // NOLINT
};
template <typename Iter>
struct IterType<Iter const*> {
  using value_type = Iter;  // NOLINT
};
template <typename Iter>
struct IterType<Iter[]> {
  using value_type = Iter;  // NOLINT
};

template <typename Iter, typename Sentinel>
constexpr auto destroy_range(Iter first, Sentinel last) noexcept;

template <typename Type>
constexpr auto destroy_in_place(Type& value) noexcept {
  if constexpr (std::is_array_v<Type>) {
    destroy_range(value, value + std::extent_v<Type>);
  } else {
    value.~Type();
  }
}

template <typename Iter, typename Sentinel>
constexpr auto destroy_range(Iter first, Sentinel last) noexcept {
  if constexpr (not std::is_trivially_destructible_v<
                    typename IterType<Iter>::value_type>) {
    for (; first != last; ++first) {
      destroy_in_place(*first);
    }
  }
}

}  // namespace detail

template <typename Type, typename Allocator = std::allocator<Type>>
  requires(std::is_same_v<Type, std::remove_reference_t<Type>>)
class ArrayList
    : private std::allocator_traits<Allocator>::template rebind_alloc<Type> {
  static_assert(
      std::is_same_v<Type, typename Allocator::value_type>,
      "Requires allocator's type to match the type held by the ArrayList");
  static_assert(std::is_object_v<Type>,
                "Requires type held by the ArrayList to be an object");

  // NOLINTBEGIN
  using Alty =
      typename std::allocator_traits<Allocator>::template rebind_alloc<Type>;
  using allocator_traits = std::allocator_traits<Alty>;

 public:
  using value_type = Type;
  using allocator_type = Allocator;
  using pointer = typename allocator_traits::pointer;
  using const_pointer = typename allocator_traits::const_pointer;
  using reference = Type&;
  using const_reference = const Type&;
  using size_type = typename allocator_traits::size_type;
  using difference_type = typename allocator_traits::difference_type;
  // NOLINTEND

  constexpr auto max_size() const noexcept -> size_type {
    return static_cast<size_type>(-1) / sizeof(value_type);
  }

 private:
  constexpr auto calculate_growth(const size_type new_size) const noexcept
      -> size_type {
    const auto old_capacity = capacity();

    if (new_size > max_size() - old_capacity / 2) {
      return max_size();
    }
    const auto growth = old_capacity + old_capacity / 2;

    if (growth < new_size) {
      return new_size;
    }
    return growth;
  }

  AL_NODISCARD constexpr inline auto get_allocator() noexcept
      -> allocator_type& {
    return static_cast<allocator_type&>(*this);
  }

  struct ConstIterator {
   public:
    // NOLINTBEGIN
    using iterator_category = std::random_access_iterator_tag;
    using value_type = value_type;
    using difference_type = difference_type;
    using pointer = pointer;
    using const_pointer = const_pointer;
    using reference = reference;
    using const_reference = const_reference;

    constexpr ConstIterator(pointer current) : current_(current) {}
    constexpr ConstIterator(std::nullptr_t) : current_(nullptr) {}
    // NOLINTEND

    AL_NODISCARD constexpr auto operator*() noexcept -> reference {
      return *current_;
    }
    AL_NODISCARD constexpr auto operator*() const noexcept -> const_reference {
      return *current_;
    }
    constexpr auto operator->() noexcept -> pointer { return current_; }
    constexpr auto operator->() const noexcept -> const_pointer {
      return current_;
    }

    constexpr auto operator++() noexcept -> ConstIterator& {
      ++current_;
      return *this;
    }
    constexpr auto operator++(int) noexcept -> ConstIterator {
      auto tmp = *this;
      ++current_;
      return tmp;
    }

    AL_NODISCARD friend constexpr auto operator not_eq(
        const ConstIterator& me, const ConstIterator& other) noexcept {
      return me.current_ not_eq other.current_;
    }
    AL_NODISCARD friend constexpr auto operator==(
        const ConstIterator& me, const ConstIterator& other) noexcept {
      return me.current_ == other.current_;
    }

    AL_NODISCARD friend constexpr auto operator-(
        const ConstIterator& me,
        const ConstIterator& other) noexcept -> difference_type {
      return static_cast<difference_type>(me.current_ - other.current_);
    }

    template <std::integral Integer>
    friend constexpr auto operator+(const ConstIterator& me,
                                    Integer n) noexcept -> ConstIterator {
      return ConstIterator(me.current_ + n);
    }

   protected:
    pointer current_;
  };

  struct Iterator : public ConstIterator {
   public:
    // NOLINTBEGIN
    using iterator_category = std::random_access_iterator_tag;
    using value_type = value_type;
    using difference_type = difference_type;
    using pointer = pointer;
    using reference = reference;

    constexpr Iterator(pointer current) : ConstIterator(current) {}
    constexpr Iterator(std::nullptr_t) : ConstIterator(nullptr) {}
    // NOLINTEND

    AL_NODISCARD constexpr auto operator*() noexcept -> reference {
      return *this->current_;
    }
    AL_NODISCARD constexpr auto operator*() const noexcept -> const_reference {
      return *this->current_;
    }
    constexpr auto operator->() noexcept -> pointer { return this->current_; }
    constexpr auto operator->() const noexcept -> const_pointer {
      return this->current_;
    }

    constexpr auto operator++() noexcept -> Iterator& {
      ++this->current_;
      return *this;
    }
    constexpr auto operator++(int) noexcept -> Iterator {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    template <std::integral Integer>
    friend constexpr auto operator+(const Iterator& me,
                                    Integer n) noexcept -> Iterator {
      return Iterator(me.current_ + n);
    }

    AL_NODISCARD friend constexpr auto operator not_eq(
        const Iterator& me, const Iterator& other) noexcept {
      return me.current_ not_eq other.current_;
    }
    AL_NODISCARD friend constexpr auto operator==(
        const Iterator& me, const Iterator& other) noexcept {
      return me.current_ == other.current_;
    }

   private:
    constexpr inline auto current() noexcept -> pointer {
      return ConstIterator::current_;
    }
    constexpr inline auto current() const noexcept -> pointer {
      return ConstIterator::current_;
    }
  };

 public:
  // NOLINTBEGIN
  using iterator = Iterator;
  using const_iterator = ConstIterator;
  // NOLINTEND

  constexpr explicit ArrayList(const size_type capacity = 8_UZ,
                               const allocator_type& alloc = allocator_type())
      : allocator_type(alloc),
        data_(allocator_traits::allocate(get_allocator(), capacity)) {
    end_ = data_ + capacity;
    current_ = data_;
  }
  template <typename Iter, std::enable_if_t<detail::IsIterator<Iter>, int> = 0>
  constexpr ArrayList(Iter first, Iter last,
                      const allocator_type& alloc = allocator_type())
      : allocator_type(alloc) {
    const auto ufirst = detail::get_unwrapped(first);
    const auto ulast = detail::get_unwrapped(last);

    const auto length = static_cast<size_t>(std::distance(ufirst, ulast));

    data_ = allocator_traits::allocate(get_allocator(), length);
    end_ = data_ + length;
    current_ = data_ + length;

    std::uninitialized_copy(ufirst, ulast, data_);
  }

  constexpr ArrayList(std::initializer_list<Type> list,
                      const allocator_type& alloc = allocator_type())
      : ArrayList(list.begin(), list.end(), alloc) {}

  template <IterableContainer Container>
  constexpr explicit ArrayList(const Container& container,
                               const allocator_type& alloc = allocator_type())
      : ArrayList(container.begin(), container.end(), alloc) {}
  template <IterableContainer Container>
  constexpr explicit ArrayList(Container&& container,
                               const allocator_type& alloc = allocator_type())
      : ArrayList(container.begin(), container.end(), alloc) {}

  constexpr ArrayList(const ArrayList& other,
                      const allocator_type& alloc = allocator_type())
      : ArrayList(other.begin(), other.end(), alloc) {}
  constexpr ArrayList(ArrayList&& other) noexcept : ArrayList(0) {
    swap_with_other(other);
  }

  constexpr auto operator=(const ArrayList& other) -> ArrayList& {
    if (this not_eq &other) {
      copy_safe(other);
    }
    return *this;
  }
  constexpr auto operator=(ArrayList&& other) noexcept -> ArrayList& {
    swap_with_other(other);

    return *this;
  }

  ~ArrayList() {
    destruct_all_elements();
    deallocate_ptr();
  }

  AL_NODISCARD constexpr inline auto empty() const noexcept -> bool {
    return data_ == current_;
  }

  template <typename Iter, std::enable_if_t<detail::IsIterator<Iter>, int> = 0>
  constexpr auto push_back(Iter first, Iter last) -> void {
    const auto ufirst = detail::get_unwrapped(first);
    const auto ulast = detail::get_unwrapped(last);

    const auto length = std::distance(ufirst, ulast);
    ensure_size_for_elements(length);

    std::uninitialized_copy(ufirst, ulast, current_);
    current_ += length;
  }

  auto push_back(const Type& value) -> void {
    ensure_size_for_elements(1_UZ);
    raw_push_back(value);
  }
  auto push_back(Type&& value) -> void {
    ensure_size_for_elements(1_UZ);
    raw_push_back(std::move(value));
  }
  template <typename... Args>
  auto emplace_back(Args&&... args) -> value_type& {
    ensure_size_for_elements(1_UZ);
    return raw_emplace_back(std::forward<Args>(args)...);
  }

  constexpr void pop_back() {
    ensure_not_empty();
    // destroy it!
    current_->~value_type();
    --current_;
  }

  AL_NODISCARD constexpr auto size() const noexcept -> size_type {
    return current_ - data_;
  }

  void resize(const size_type new_size)
    requires(std::is_constructible_v<value_type>)
  {
    const auto len = size();
    if (new_size < len) {
      for (auto i = new_size; i < len; ++i) {
        data_[i].~value_type();
      }
    }

    if (capacity() < new_size) {
      reserve(new_size);
    }
    auto* tmp = current_;
    current_ = data_ + new_size;

    for (; tmp < current_; ++tmp) {
      new (tmp) value_type();
    }
  }

  void reserve(const size_type new_capacity) {
    const auto cap = capacity();
    if (cap >= new_capacity) {
      return;
    }

    const auto len = size();
    const auto old_ptr = data_;
    raw_reserve(new_capacity);
    if (old_ptr) {
      if (len > 0) {
        std::memcpy(data_, old_ptr, len * sizeof(Type));
        // std::uninitialized_copy(old_ptr, old_ptr + len, data_);
      }
      if (cap > 0) {
        deallocate_target_ptr(old_ptr, cap);
      }
    }
  }

  AL_NODISCARD constexpr auto operator[](const size_type index) noexcept
      -> reference {
    return data_[index];
  }
  AL_NODISCARD constexpr auto operator[](const size_type index) const noexcept
      -> const_reference {
    return data_[index];
  }
  AL_NODISCARD constexpr auto at(const size_type index) -> reference {
    ensure_in_range(index);
    return data_[index];
  }
  AL_NODISCARD constexpr auto at(const size_type index) const
      -> const_reference {
    ensure_in_range(index);
    return data_[index];
  }

  AL_NODISCARD constexpr auto capacity() const noexcept -> size_type {
    return end_ - data_;
  }

  AL_NODISCARD constexpr auto data() noexcept -> pointer { return data_; }
  AL_NODISCARD constexpr auto data() const noexcept -> const_pointer {
    return data_;
  }

  AL_NODISCARD constexpr auto front() -> reference {
    ensure_not_empty();
    return *data_;
  }
  AL_NODISCARD constexpr auto front() const -> const_reference {
    ensure_not_empty();
    return *data_;
  }

  AL_NODISCARD constexpr auto back() -> reference {
    ensure_not_empty();
    return *(current_ - 1);
  }
  AL_NODISCARD constexpr auto back() const -> const_reference {
    ensure_not_empty();
    return *(current_ - 1);
  }

  AL_NODISCARD constexpr auto begin() noexcept -> iterator { return data_; }
  AL_NODISCARD constexpr auto end() noexcept -> iterator { return current_; }
  AL_NODISCARD constexpr auto cbegin() noexcept -> iterator { return data_; }
  AL_NODISCARD constexpr auto cend() noexcept -> iterator { return current_; }
  AL_NODISCARD constexpr auto begin() const noexcept -> const_iterator {
    return data_;
  }
  AL_NODISCARD constexpr auto end() const noexcept -> const_iterator {
    return current_;
  }
  AL_NODISCARD constexpr auto cbegin() const noexcept -> const_iterator {
    return data_;
  }
  AL_NODISCARD constexpr auto cend() const noexcept -> const_iterator {
    return current_;
  }

  constexpr auto clear() noexcept -> void {
    destruct_all_elements();
    current_ = data_;
  }

  constexpr auto erase(const_iterator position) -> iterator {
    const auto index = static_cast<size_type>(position - begin());
    erase(index);
    return begin() + index;
  }

  constexpr auto erase(const size_type index) -> iterator {
    if (index >= size() or size() == 0) {
      throw std::out_of_range("Index out of range");
    }
    const auto length = size();
    for (auto i = index; i < length - 1; ++i) {
      data_[i] = std::move(data_[i + 1]);
    }
    --current_;
    return data_ + index;
  }

 private:
  constexpr void raw_reserve(const size_type capacity) {
    const auto length = size();
    data_ = allocator_traits::allocate(get_allocator(), capacity);
    current_ = data_ + length;
    end_ = data_ + capacity;
  }

  constexpr void copy_safe(const ArrayList& other) {
    destruct_all_elements();
    deallocate_ptr();

    reserve(other.size());
    copy_unsafe(other);
  }

  constexpr void copy_unsafe(const ArrayList& other) {
    const auto length{other.size()};
    current_ = data_ + length;
    for (auto i = 0_UZ; i < length; ++i) {
      new (data_ + i) value_type(other.data_[i]);
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
  void grow_capacity() {
    const auto new_capacity = capacity() + 1;
    const auto growth = calculate_growth(new_capacity);
    reserve(growth);
  }

  template <typename... Args>
  constexpr auto raw_emplace_back(Args&&... args) -> value_type& {
    return emplace_at_back(std::forward<Args>(args)...);
  }
  constexpr auto raw_push_back(const Type& value) -> void {
    push_at_back(value);
  }
  constexpr auto raw_push_back(Type&& value) -> void {
    push_at_back(std::move(value));
  }

  template <typename... Args>
  constexpr auto raw_emplace_into(value_type* const my_ptr,
                                  Args&&... args) -> value_type& {
    new (my_ptr) value_type(std::forward<Args>(args)...);
    return *my_ptr;
  }
  constexpr auto raw_push_into(value_type* const my_ptr,
                               const Type& value) -> void {
    new (my_ptr) value_type(value);
  }
  constexpr auto raw_push_into(value_type* const my_ptr, Type&& value) -> void {
    new (my_ptr) value_type(std::move(value));
  }

  template <typename... Args>
  constexpr auto emplace_at_back(Args&&... args) -> value_type& {
    return raw_emplace_into(current_++, std::forward<Args>(args)...);
  }
  constexpr auto push_at_back(const Type& value) -> void {
    raw_push_into(current_++, value);
  }
  constexpr auto push_at_back(Type&& value) -> void {
    raw_push_into(current_++, std::move(value));
  }

  constexpr inline auto destruct_all_elements() -> void {
    if constexpr (not std::is_trivially_destructible_v<Type>) {
      detail::destroy_range(data_, current_);
    }
  }
  constexpr auto deallocate_target_ptr(value_type* const ptr,
                                       const size_type length) -> void {
    if (ptr) {
      allocator_traits::deallocate(get_allocator(), ptr, length);
    }
  }
  constexpr auto deallocate_ptr() -> void {
    deallocate_target_ptr(data(), capacity());
  }

  constexpr auto swap_with_other(ArrayList& other) -> void {
    std::swap(data_, other.data_);
    std::swap(end_, other.end_);
    std::swap(current_, other.current_);
  }

  pointer data_;
  pointer end_;
  pointer current_;
};

}  // namespace al

namespace std {
template <typename Type, typename Ally>
constexpr auto distance(
    const typename al::ArrayList<Type, Ally>::iterator first,
    const typename al::ArrayList<Type, Ally>::iterator second)
    -> al::ArrayList<Type, Ally>::size_type {
  using MySizeType = al::ArrayList<Type, Ally>::size_type;
  return static_cast<MySizeType>(second - first);
}
template <typename Type, typename Ally>
constexpr auto distance(
    const typename al::ArrayList<Type, Ally>::const_iterator first,
    const typename al::ArrayList<Type, Ally>::const_iterator second)
    -> al::ArrayList<Type, Ally>::size_type {
  using MySizeType = al::ArrayList<Type, Ally>::size_type;
  return static_cast<MySizeType>(second - first);
}
}  // namespace std

#endif  // ARRAY_LIST_HPP
