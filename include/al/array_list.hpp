#ifndef ARRAY_LIST_HPP
#define ARRAY_LIST_HPP

#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#define HAS_CXX20 (__cplusplus >= 202002UL)
#define HAS_CONCEPTS (HAS_CXX20)

#if HAS_CONCEPTS
#include <concepts>
#endif

#if HAS_CONCEPTS
#define CONSTRAINT(CONSTRAINT_NAME) CONSTRAINT_NAME
#else
#define CONSTRAINT(X) typename
#endif

namespace al {

#if HAS_CXX20
using std::remove_cvref_t;
#else   // ^^^ HAS_CXX20
// NOLINTBEGIN
template <typename Type>
using remove_cvref_t =
    typename std::remove_cv<typename std::remove_reference<Type>::type>::type;
// NOLINTEND
#endif  // ^^^ !HAS_CXX20

#ifdef AL_NODISCARD
#undef AL_NODISCARD
#endif
#define AL_NODISCARD [[nodiscard]]

#if HAS_CONCEPTS
template <typename Container>
concept IterableContainer = requires(Container c) {
  std::begin(c);
  std::end(c);
};
#endif  // ^^^ HAS_CONCEPTS

constexpr auto operator""_UZ(const unsigned long long value) -> size_t {
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
    std::void_t<decltype(std::declval<al::remove_cvref_t<Iter>&>()._Seek_to(
        std::declval<Iter>()._Unwrapped()))>> =
    AllowInheritingUnwrap<al::remove_cvref_t<Iter>>;

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

template <typename ArrayList>
struct ArrayListConstIterator {
 public:
  // NOLINTBEGIN
  using iterator_category = std::random_access_iterator_tag;
  using value_type = typename ArrayList::value_type;
  using difference_type = ptrdiff_t;
  using pointer = const value_type*;
  using reference = const value_type&;

  constexpr ArrayListConstIterator(value_type* const current) : ptr_(current) {}
  constexpr ArrayListConstIterator(std::nullptr_t) : ptr_(nullptr) {}
  // NOLINTEND

  AL_NODISCARD constexpr auto operator*() const noexcept -> reference {
    return *ptr_;
  }
  constexpr auto operator->() const noexcept -> pointer { return ptr_; }

  constexpr auto operator++() noexcept -> ArrayListConstIterator& {
    ++ptr_;
    return *this;
  }
  constexpr auto operator++(int) noexcept -> ArrayListConstIterator {
    const auto tmp = *this;
    this->operator++();
    return tmp;
  }
  constexpr auto operator--() noexcept -> ArrayListConstIterator& {
    --ptr_;
    return *this;
  }
  constexpr auto operator--(int) noexcept -> ArrayListConstIterator {
    const auto tmp = *this;
    this->operator--();
    return tmp;
  }

  AL_NODISCARD constexpr auto operator!=(
      const ArrayListConstIterator& other) const& noexcept {
    return ptr_ != other.ptr_;
  }
  AL_NODISCARD constexpr auto operator==(
      const ArrayListConstIterator& other) const& noexcept {
    return ptr_ == other.ptr_;
  }

  AL_NODISCARD constexpr auto operator-(
      const ArrayListConstIterator& other) const& noexcept -> difference_type {
    return static_cast<difference_type>(ptr_ - other.ptr_);
  }

  template <CONSTRAINT(std::integral) Integer>
  constexpr auto operator+(Integer n) const& noexcept
      -> ArrayListConstIterator {
    return ArrayListConstIterator(ptr_ + n);
  }

 protected:
  value_type* ptr_;
};

template <typename ArrayList>
struct ArrayListIterator : public ArrayListConstIterator<ArrayList> {
 private:
  using Base = ArrayListConstIterator<ArrayList>;

 public:
  // NOLINTBEGIN
  using iterator_category = std::random_access_iterator_tag;
  using value_type = typename Base::value_type;
  using difference_type = typename Base::difference_type;
  using pointer = value_type*;
  using reference = value_type&;

  constexpr ArrayListIterator(pointer current)
      : ArrayListConstIterator<ArrayList>(current) {}
  constexpr ArrayListIterator(std::nullptr_t)
      : ArrayListConstIterator<ArrayList>(nullptr) {}
  // NOLINTEND

  AL_NODISCARD constexpr auto operator*() const noexcept -> reference {
    return *Base::ptr_;
  }
  constexpr auto operator->() const noexcept -> pointer { return Base::ptr_; }

  constexpr auto operator++() noexcept -> ArrayListIterator& {
    Base::operator++();
    return *this;
  }
  constexpr auto operator++(int) noexcept -> ArrayListIterator {
    const auto tmp = *this;
    Base::operator++();
    return tmp;
  }
  constexpr auto operator--() noexcept -> ArrayListIterator& {
    Base::operator--();
    return *this;
  }
  constexpr auto operator--(int) noexcept -> ArrayListIterator {
    const auto tmp = *this;
    Base::operator--();
    return *this;
  }

  template <CONSTRAINT(std::integral) Integer>
  constexpr auto operator+(Integer n) noexcept -> ArrayListIterator {
    return ArrayListIterator(Base::ptr_ + n);
  }

  AL_NODISCARD constexpr auto operator!=(
      const ArrayListIterator& other) const& noexcept {
    return Base::operator!=(other);
  }
  AL_NODISCARD constexpr auto operator==(
      const ArrayListIterator& other) const& noexcept {
    return Base::operator==(other);
  }
};

template <typename Type, typename Allocator = std::allocator<Type>>
#if HAS_CONCEPTS
  requires(std::is_same_v<Type, std::remove_reference_t<Type>>)
#endif
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
  using iterator = ArrayListIterator<ArrayList>;
  using const_iterator = ArrayListConstIterator<ArrayList>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
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

 public:
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

  template <CONSTRAINT(IterableContainer) Container>
  constexpr explicit ArrayList(const Container& container,
                               const allocator_type& alloc = allocator_type())
      : ArrayList(container.begin(), container.end(), alloc) {}

  constexpr ArrayList(const ArrayList& other,
                      const allocator_type& alloc = allocator_type())
      : ArrayList(other.begin(), other.end(), alloc) {}
  constexpr ArrayList(ArrayList&& other) noexcept : ArrayList(0) {
    swap_with_other(other);
  }

  constexpr auto operator=(const ArrayList& other) -> ArrayList& {
    if (this not_eq std::addressof(other)) {
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
#if HAS_CONCEPTS
    requires(std::is_constructible_v<value_type>)
#endif
  {
    const auto len = size();
    if (new_size < len) {
      // for (auto i = new_size; i < len; ++i) {
      //   data_[i].~value_type();
      // }
      std::destroy_n(data_ + new_size, len - new_size);
    }

    if (capacity() < new_size) {
      reserve(new_size);
    }
    auto* tmp = current_;
    current_ = data_ + new_size;

    // for (; tmp < current_; ++tmp) {
    //   new (tmp) value_type();
    // }
    std::uninitialized_value_construct_n(tmp, new_size);
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

  AL_NODISCARD constexpr auto begin() noexcept -> iterator {
    return iterator(data_);
  }
  AL_NODISCARD constexpr auto end() noexcept -> iterator {
    return iterator(current_);
  }
  AL_NODISCARD constexpr auto cbegin() noexcept -> iterator {
    return iterator(data_);
  }
  AL_NODISCARD constexpr auto cend() noexcept -> iterator {
    return iterator(current_);
  }
  AL_NODISCARD constexpr auto begin() const noexcept -> const_iterator {
    return const_iterator(data_);
  }
  AL_NODISCARD constexpr auto end() const noexcept -> const_iterator {
    return const_iterator(current_);
  }
  AL_NODISCARD constexpr auto cbegin() const noexcept -> const_iterator {
    return const_iterator(data_);
  }
  AL_NODISCARD constexpr auto cend() const noexcept -> const_iterator {
    return const_iterator(current_);
  }
  AL_NODISCARD constexpr auto rbegin() noexcept -> reverse_iterator {
    return reverse_iterator(end());
  }
  AL_NODISCARD constexpr auto rend() noexcept -> reverse_iterator {
    return reverse_iterator(begin());
  }
  AL_NODISCARD constexpr auto rbegin() const noexcept
      -> const_reverse_iterator {
    return const_reverse_iterator(end());
  }
  AL_NODISCARD constexpr auto rend() const noexcept -> const_reverse_iterator {
    return const_reverse_iterator(begin());
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
    // for (auto i = 0_UZ; i < length; ++i) {
    //   new (data_ + i) value_type(other.data_[i]);
    // }
    std::uninitialized_copy_n(other.data_, length, data_);
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
    const typename al::ArrayList<Type, Ally>::iterator second) ->
    typename al::ArrayList<Type, Ally>::size_type {
  using MySizeType = typename al::ArrayList<Type, Ally>::size_type;
  return static_cast<MySizeType>(second - first);
}
template <typename Type, typename Ally>
constexpr auto distance(
    const typename al::ArrayList<Type, Ally>::const_iterator first,
    const typename al::ArrayList<Type, Ally>::const_iterator second) ->
    typename al::ArrayList<Type, Ally>::size_type {
  using MySizeType = typename al::ArrayList<Type, Ally>::size_type;
  return static_cast<MySizeType>(second - first);
}
}  // namespace std

#endif  // ARRAY_LIST_HPP
