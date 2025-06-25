#ifndef ARRAY_LIST_HPP
#define ARRAY_LIST_HPP

#define HAS_CXX20 (__cplusplus >= 202002UL)
#define HAS_CXX17 (__cplusplus >= 201703UL)

#if !HAS_CXX17
#error "This library requires C++17 or higher"
#endif

#if HAS_CXX17
#define CONSTEXPR_CXX17 constexpr
#else
#define CONSTEXPR_CXX17 inline
#endif

#if HAS_CXX20
#define CONSTEXPR_CXX20 constexpr
#else
#define CONSTCONSTEXPR_CXX20
#endif

#define HAS_CONCEPTS (HAS_CXX20)

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if HAS_CONCEPTS
#include <concepts>
#endif

#if HAS_CONCEPTS
#define CONSTRAINT(CONSTRAINT_NAME) CONSTRAINT_NAME
#else
#define CONSTRAINT(X) typename
#endif

#if (defined(__clang__) && !defined(_MSC_VER))
#define CLANG 1
#else
#define CLANG 0
#endif

#if defined(__GNUC__)
#define GCC 1
#else
#define GCC 0
#endif

#if defined(_MSC_VER)
#define MSVC 1
#else
#define MSVC 0
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
// ^^^ MSVC iterators can be weird, might need something with this in future

template <class Iter, class = void>
constexpr bool IsUnwrappable = false;

template <class Iter>
constexpr bool IsUnwrappable<
    Iter, std::void_t<decltype(std::declval<al::remove_cvref_t<Iter>&>())>> =
    AllowInheritingUnwrap<al::remove_cvref_t<Iter>>;

template <class Iter, class = void>
constexpr bool HasNothrowUnwrapped = false;
template <class Iter>
constexpr bool
    HasNothrowUnwrapped<Iter, std::void_t<decltype(std::declval<Iter>())>> =
        noexcept(std::declval<Iter>());

template <class Iter>
constexpr auto get_unwrapped(Iter&& it) noexcept(not IsUnwrappable<Iter> or
                                                 HasNothrowUnwrapped<Iter>)
    -> decltype(auto) {
  if constexpr (std::is_pointer_v<std::decay_t<Iter>>) {
    return it + 0;
  } else if constexpr (IsUnwrappable<Iter>) {
    return static_cast<Iter&&>(it);
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

<<<<<<< HEAD
=======
template <typename Iter, typename Sentinel>
constexpr auto destroy_range(Iter first, Sentinel last) noexcept;

template <typename Type>
constexpr auto destroy_in_place(Type* value) noexcept {
  if constexpr (std::is_array_v<Type>) {
    for (auto& element : *value) {
      (destroy_in_place)(std::addressof(element));
    }
  } else {
    value->~Type();
  }
}

template <typename Iter, typename Sentinel>
constexpr auto destroy_range(Iter first, Sentinel last) noexcept {
  if constexpr (not std::is_trivially_destructible_v<
                    typename IterType<Iter>::value_type>) {
    for (; first != last; ++first) {
      destroy_in_place(std::addressof(*first));
    }
  }
}

>>>>>>> 671d0a717e46069a5bbf520cc3cc9267bb90938c
}  // namespace detail

template <typename ArrayList>
struct ArrayListConstIterator {
 private:
  using Self = ArrayListConstIterator;

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
    auto tmp = *this;
    Self::operator++();
    return tmp;
  }
  constexpr auto operator--() noexcept -> ArrayListConstIterator& {
    --ptr_;
    return *this;
  }
  constexpr auto operator--(int) noexcept -> ArrayListConstIterator {
    auto tmp = *this;
    Self::operator--();
    return tmp;
  }

  AL_NODISCARD constexpr auto operator!=(
      const ArrayListConstIterator& other) const& noexcept {
    return ptr_ != other.ptr_;
  }
  AL_NODISCARD constexpr auto operator==(
      const ArrayListConstIterator& other) const& noexcept {
    return !Self::operator!=(other);
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
  using Self = ArrayListIterator;
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
    return const_cast<reference>(Base::operator*());
  }
  constexpr auto operator->() const noexcept -> pointer {
    return const_cast<pointer>(Base::operator->());
  }

  constexpr auto operator++() noexcept -> ArrayListIterator& {
    Base::operator++();
    return *this;
  }
  constexpr auto operator++(int) noexcept -> ArrayListIterator {
    auto tmp = *this;
    Self::operator++();
    return tmp;
  }
  constexpr auto operator--() noexcept -> ArrayListIterator& {
    Base::operator--();
    return *this;
  }
  constexpr auto operator--(int) noexcept -> ArrayListIterator {
    auto tmp = *this;
    Self::operator--();
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
    return !Self::operator!=(other);
  }
};

/// \deprecated
/// This is the difference in size. old capacity + this value is the new
/// prefered size
struct ArrayListDefaultGrowthDifference {
  constexpr auto operator()(const size_t old_capacity) -> size_t {
#if CLANG || GCC
    // return old_capacity;
    return old_capacity / 2;
#elif MSVC
    return old_capacity / 2;
#else
    return old_capacity / 2;
#endif
  }
};

template <typename Type, typename Allocator = std::allocator<Type>>
#if HAS_CONCEPTS
  requires(std::is_same_v<Type, std::remove_reference_t<Type>>)
#endif
class ArrayListImpl {  // NOLINT
  static_assert(
      std::is_same_v<Type, typename Allocator::value_type>,
      "Requires allocator's type to match the type held by the ArrayList");
  static_assert(std::is_object_v<Type>,
                "Requires type held by the ArrayList to be an object");

  // NOLINTBEGIN
  using Alty =
      typename std::allocator_traits<Allocator>::template rebind_alloc<Type>;
  using AltyTraits = std::allocator_traits<Alty>;

 public:
  using value_type = Type;
  using allocator_type = Alty;
  using pointer = typename AltyTraits::pointer;
  using const_pointer = typename AltyTraits::const_pointer;
  using reference = Type&;
  using const_reference = const Type&;
  using size_type = typename AltyTraits::size_type;
  using difference_type = typename AltyTraits::difference_type;
  using iterator = ArrayListIterator<ArrayListImpl>;
  using const_iterator = ArrayListConstIterator<ArrayListImpl>;
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
<<<<<<< HEAD

    const auto growth = old_capacity + old_capacity / 2;
=======
    const auto growth = old_capacity + (old_capacity / 2);
>>>>>>> 671d0a717e46069a5bbf520cc3cc9267bb90938c

    if (growth < new_size) {
      return new_size;
    }
    return growth;
  }

<<<<<<< HEAD
  AL_NODISCARD constexpr inline auto get_allocator() noexcept
      -> allocator_type& {
    return compressed_.get_allocator();
=======
  AL_NODISCARD constexpr auto get_allocator() noexcept -> allocator_type& {
    return compressed_;
>>>>>>> 671d0a717e46069a5bbf520cc3cc9267bb90938c
  }

 public:
  constexpr ArrayListImpl() noexcept : compressed_() {}

  CONSTEXPR_CXX20 explicit ArrayListImpl(
      const size_type capacity, const allocator_type& alloc = allocator_type())
      : compressed_(AltyTraits::allocate(get_allocator(), capacity), alloc) {
    compressed_.end = compressed_.data + capacity;
    compressed_.current = compressed_.data;
  }

  // NOLINTBEGIN
  template <typename Iter, std::enable_if_t<detail::IsIterator<Iter>, int> = 0>
<<<<<<< HEAD
  CONSTEXPR_CXX20 ArrayListImpl(Iter first, Iter last,
                                const allocator_type& alloc = allocator_type())
=======
  // NOLINTEND
  constexpr _ArrayList_impl(Iter first, Iter last,
                            const allocator_type& alloc = allocator_type())
>>>>>>> 671d0a717e46069a5bbf520cc3cc9267bb90938c
      : compressed_(alloc) {
    const auto ufirst = detail::get_unwrapped(first);
    const auto ulast = detail::get_unwrapped(last);

    const auto length = static_cast<size_t>(std::distance(ufirst, ulast));

    compressed_.data = AltyTraits::allocate(get_allocator(), length);
    compressed_.end = compressed_.data + length;
    compressed_.current = compressed_.data + length;

    std::uninitialized_copy(ufirst, ulast, compressed_.data);
  }

  CONSTEXPR_CXX20 ArrayListImpl(std::initializer_list<Type> list,
                                const allocator_type& alloc = allocator_type())
      : ArrayListImpl(list.begin(), list.end(), alloc) {}

  template <CONSTRAINT(IterableContainer) Container>
  CONSTEXPR_CXX20 explicit ArrayListImpl(
      const Container& container,
      const allocator_type& alloc = allocator_type())
      : ArrayListImpl(container.begin(), container.end(), alloc) {}

  CONSTEXPR_CXX20 ArrayListImpl(const ArrayListImpl& other,
                                const allocator_type& alloc = allocator_type())
      : ArrayListImpl(other.begin(), other.end(), alloc) {}
  constexpr ArrayListImpl(ArrayListImpl&& other) noexcept
      : compressed_(std::exchange(other.compressed_, Compressed())) {}

  CONSTEXPR_CXX20 auto operator=(const ArrayListImpl& other) -> ArrayListImpl& {
    if (this not_eq std::addressof(other)) {
      copy_safe(other);
    }
    return *this;
  }
  constexpr auto operator=(ArrayListImpl&& other) noexcept -> ArrayListImpl& {
    destruct_all_elements();
    deallocate_ptr();

    compressed_ = std::exchange(other.compressed_, Compressed());

    return *this;
  }

  CONSTEXPR_CXX20 ~ArrayListImpl() {
    destruct_all_elements();
    deallocate_ptr();
  }

  AL_NODISCARD constexpr auto empty() const noexcept -> bool {
    return compressed_.data == compressed_.current;
  }

  // NOLINTBEGIN
  template <typename Iter, std::enable_if_t<detail::IsIterator<Iter>, int> = 0>
  // NOLINTEND
  constexpr auto push_back(Iter first, Iter last) -> void {
    const auto ufirst = detail::get_unwrapped(first);
    const auto ulast = detail::get_unwrapped(last);

    const auto length = std::distance(ufirst, ulast);
    ensure_size_for_elements(length);

    std::uninitialized_copy(ufirst, ulast, compressed_.current);
    compressed_.current += length;
  }

  CONSTEXPR_CXX20 auto push_back(const Type& value) -> void {
    ensure_size_for_elements(1_UZ);
    raw_push_back(value);
  }
  CONSTEXPR_CXX20 auto push_back(Type&& value) -> void {
    ensure_size_for_elements(1_UZ);
    raw_push_back(std::move(value));
  }
  template <typename... Args>
  CONSTEXPR_CXX20 auto emplace_back(Args&&... args) -> value_type& {
    ensure_size_for_elements(1_UZ);
    return raw_emplace_back(std::forward<Args>(args)...);
  }

  CONSTEXPR_CXX20 void pop_back() {
    ensure_not_empty();
    // destroy it!
    compressed_.current->~value_type();
    --compressed_.current;
  }

  AL_NODISCARD constexpr auto size() const noexcept -> size_type {
    return compressed_.current - compressed_.data;
  }

  CONSTEXPR_CXX20 void resize(const size_type new_size)
#if HAS_CONCEPTS
    requires(std::is_constructible_v<value_type>)
#endif
  {
    const auto len = size();
    if (new_size < len) {
      std::destroy_n(compressed_.data + new_size, len - new_size);
    }

    if (capacity() < new_size) {
      reserve(new_size);
    }
    auto* tmp = compressed_.current;
    compressed_.current = compressed_.data + new_size;

    std::uninitialized_value_construct_n(tmp, compressed_.current - tmp);
  }

  CONSTEXPR_CXX20 void reserve(const size_type new_capacity) {
    const auto cap = capacity();
    if (cap >= new_capacity) {
      return;
    }

    const auto len = size();
    const auto old_ptr = compressed_.data;
    raw_reserve(new_capacity);
    if (old_ptr) {
      if (cap > 0) {
        if (len > 0) {
          std::uninitialized_move_n(old_ptr, len, compressed_.data);
        }
<<<<<<< HEAD
        destroy_range(get_allocator(), old_ptr, old_ptr + len);
=======
        ::al::detail::destroy_range(old_ptr, old_ptr + len);
>>>>>>> 671d0a717e46069a5bbf520cc3cc9267bb90938c
        deallocate_target_ptr(old_ptr, cap);
      }
    }
  }

  AL_NODISCARD constexpr auto operator[](const size_type index) noexcept
      -> reference {
    return compressed_.data[index];
  }
  AL_NODISCARD constexpr auto operator[](const size_type index) const noexcept
      -> const_reference {
    return compressed_.data[index];
  }
  AL_NODISCARD constexpr auto at(const size_type index) -> reference {
    ensure_in_range(index);
    return compressed_.data[index];
  }
  AL_NODISCARD constexpr auto at(const size_type index) const
      -> const_reference {
    ensure_in_range(index);
    return compressed_.data[index];
  }

  AL_NODISCARD constexpr auto capacity() const noexcept -> size_type {
    return compressed_.end - compressed_.data;
  }

  AL_NODISCARD constexpr auto data() noexcept -> pointer {
    return compressed_.data;
  }
  AL_NODISCARD constexpr auto data() const noexcept -> const_pointer {
    return compressed_.data;
  }

  AL_NODISCARD constexpr auto front() -> reference {
    ensure_not_empty();
    return *compressed_.data;
  }
  AL_NODISCARD constexpr auto front() const -> const_reference {
    ensure_not_empty();
    return *compressed_.data;
  }

  AL_NODISCARD constexpr auto back() -> reference {
    ensure_not_empty();
    return *(compressed_.current - 1);
  }
  AL_NODISCARD constexpr auto back() const -> const_reference {
    ensure_not_empty();
    return *(compressed_.current - 1);
  }

  AL_NODISCARD constexpr auto begin() noexcept -> iterator {
    return iterator(compressed_.data);
  }
  AL_NODISCARD constexpr auto end() noexcept -> iterator {
    return iterator(compressed_.current);
  }
  AL_NODISCARD constexpr auto begin() const noexcept -> const_iterator {
    return const_iterator(compressed_.data);
  }
  AL_NODISCARD constexpr auto end() const noexcept -> const_iterator {
    return const_iterator(compressed_.current);
  }
  AL_NODISCARD constexpr auto cbegin() const noexcept -> const_iterator {
    return const_iterator(compressed_.data);
  }
  AL_NODISCARD constexpr auto cend() const noexcept -> const_iterator {
    return const_iterator(compressed_.current);
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
  AL_NODISCARD constexpr auto crbegin() const noexcept
      -> const_reverse_iterator {
    return rbegin();
  }
  AL_NODISCARD constexpr auto crend() const noexcept -> const_reverse_iterator {
    return rend();
  }

  CONSTEXPR_CXX20 auto clear() noexcept -> void {
    destruct_all_elements();
    compressed_.current = compressed_.data;
  }

  CONSTEXPR_CXX20 auto erase(const_iterator position) -> iterator {
    const auto index = static_cast<size_type>(position - begin());
    erase(index);
    return begin() + index;
  }

  CONSTEXPR_CXX20 auto erase(const size_type index) -> iterator {
    if (index >= size() or size() == 0) {
      throw std::out_of_range("Index out of range");
    }
    const auto length = size();
    std::move(compressed_.data + index + 1, compressed_.data + length + 1,
              compressed_.data + index);
    --compressed_.current;
    return compressed_.data + index;
  }

 private:
  CONSTEXPR_CXX20 void raw_reserve(const size_type capacity) {
    const auto length = size();
    compressed_.data = AltyTraits::allocate(get_allocator(), capacity);
    compressed_.current = compressed_.data + length;
    compressed_.end = compressed_.data + capacity;
  }

  CONSTEXPR_CXX20 void copy_safe(const ArrayListImpl& other) {
    destruct_all_elements();
    deallocate_ptr();

    reserve(other.size());
    copy_unsafe(other);
  }

  CONSTEXPR_CXX20 void copy_unsafe(const ArrayListImpl& other) {
    const auto length{other.size()};
    compressed_.current = compressed_.data + length;
    std::uninitialized_copy_n(other.compressed_.data, length, compressed_.data);
  }

  constexpr void ensure_in_range(const size_type index) const {
    if (index >= size() or compressed_.data == nullptr) {
      throw std::out_of_range("Index out of range");
    }
  }

  constexpr void ensure_not_empty() const {
    if (size() <= 0_UZ or compressed_.data == nullptr) {
      throw std::out_of_range("ArrayList is empty");
    }
  }

  CONSTEXPR_CXX20 void ensure_size_for_elements(const size_type elements) {
    if (!(size() + elements <= capacity())) {
      auto new_capacity = calculate_growth(capacity() + elements);
      reserve(new_capacity);
    }
  }
  CONSTEXPR_CXX20 void grow_capacity() { return ensure_size_for_elements(1); }

  template <typename... Args>
  CONSTEXPR_CXX20 auto raw_emplace_back(Args&&... args) -> value_type& {
    return emplace_at_back(std::forward<Args>(args)...);
  }
  CONSTEXPR_CXX20 auto raw_push_back(const Type& value) -> void {
    push_at_back(value);
  }
  CONSTEXPR_CXX20 auto raw_push_back(Type&& value) -> void {
    push_at_back(std::move(value));
  }

  template <typename... Args>
  constexpr auto raw_emplace_into(value_type* const my_ptr, Args&&... args)
      -> value_type& {
    new (my_ptr) value_type(std::forward<Args>(args)...);
    return *my_ptr;
  }
  constexpr auto raw_push_into(value_type* const my_ptr, const Type& value)
      -> void {
    new (my_ptr) value_type(value);
  }
  CONSTEXPR_CXX20 auto raw_push_into(value_type* const my_ptr, Type&& value)
      -> void {
    AltyTraits::construct(get_allocator(), my_ptr, std::move(value));
  }

  template <typename... Args>
  CONSTEXPR_CXX20 auto emplace_at_back(Args&&... args) -> value_type& {
    return raw_emplace_into(compressed_.current++, std::forward<Args>(args)...);
  }
  CONSTEXPR_CXX20 auto push_at_back(const Type& value) -> void {
    raw_push_into(compressed_.current++, value);
  }
  CONSTEXPR_CXX20 auto push_at_back(Type&& value) -> void {
    raw_push_into(compressed_.current++, std::move(value));
  }

  CONSTEXPR_CXX20 auto destroy_in_place(allocator_type& alloc,
                                        Type* value) noexcept {
    if constexpr (std::is_array_v<Type>) {
      for (auto& element : *value) {
        (destroy_in_place)(alloc, std::addressof(element));
      }
    } else {
      AltyTraits::destroy(alloc, value);
    }
  }

<<<<<<< HEAD
  template <typename It, typename Sentinel>
  CONSTEXPR_CXX20 auto destroy_range(allocator_type& alloc, It first,
                                     Sentinel last) {
    if constexpr (not std::is_trivially_destructible_v<Type>) {
      for (; first != last; ++first) {
        destroy_in_place(alloc, std::addressof(*first));
      }
    }
  }

  CONSTEXPR_CXX20 auto destruct_all_elements() -> void {
    if constexpr (not std::is_trivially_destructible_v<Type>) {
      destroy_range(get_allocator(), compressed_.data, compressed_.current);
=======
  CONSTEXPR_CXX20 auto destruct_all_elements() -> void {
    if constexpr (not std::is_trivially_destructible_v<Type>) {
      ::al::detail::destroy_range(compressed_.data, compressed_.current);
>>>>>>> 671d0a717e46069a5bbf520cc3cc9267bb90938c
    }
  }
  CONSTEXPR_CXX20 auto deallocate_target_ptr(value_type* const ptr,
                                             const size_type length) -> void {
    if (ptr) {
      AltyTraits::deallocate(get_allocator(), ptr, length);
    }
  }
  CONSTEXPR_CXX20 auto deallocate_ptr() -> void {
    deallocate_target_ptr(data(), capacity());
  }

  constexpr auto swap_with_other(ArrayListImpl& other) -> void {
    std::swap(compressed_, other.compressed_);
  }

  struct Compressed : public allocator_type {
    constexpr Compressed(pointer data, pointer end, pointer current)
        : data(data), end(end), current(current) {}
    constexpr explicit Compressed(pointer data,
                                  const allocator_type& ally = allocator_type())
        : allocator_type(ally), data(data), end(data), current(data) {}
    constexpr Compressed() = default;
    constexpr explicit Compressed(const allocator_type& ally)
        : allocator_type(ally) {}

    constexpr auto get_allocator() noexcept -> allocator_type& {
      return static_cast<allocator_type&>(*this);
    }

    pointer data = nullptr;
    pointer end = nullptr;
    pointer current = nullptr;
  };

  Compressed compressed_;
};

template <typename Type, typename Allocator = std::allocator<Type>>
using ArrayList = ArrayListImpl<Type, Allocator>;

}  // namespace al

#endif  // ARRAY_LIST_HPP
