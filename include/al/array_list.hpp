#ifndef ARRAY_LIST_HPP
#define ARRAY_LIST_HPP

#ifdef _MSVC_LANG
#define AL_CPP_DEF _MSVC_LANG
#else
#define AL_CPP_DEF __cplusplus
#endif

#define AL_HAS_CXX20 (AL_CPP_DEF >= 202002L)
#define AL_HAS_CXX17 (AL_CPP_DEF >= 201703L)
#define AL_HAS_CXX14 (AL_CPP_DEF >= 201402L)
#define AL_HAS_CXX11 (AL_CPP_DEF >= 201103L)

#if !AL_HAS_CXX11
#error "This library AL_requires C++11 or higher"
#endif

#if AL_HAS_CXX17
#define AL_CONSTEXPR_CXX17 constexpr
#else
#define AL_CONSTEXPR_CXX17
#endif

#if AL_HAS_CXX20
#define AL_CONSTEXPR_CXX20 constexpr
#else
#define AL_CONSTEXPR_CXX20
#endif

#define AL_HAS_CONCEPTS (__cpp_concepts >= 201907UL)

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if AL_HAS_CONCEPTS
#include <concepts>  // IWYU pragma: keep
#endif

#if AL_HAS_CONCEPTS
#define AL_CONSTRAINT(CONSTRAINT_NAME) CONSTRAINT_NAME
#define AL_REQUIRES(...) requires(__VA_ARGS__)
#else
#define AL_CONSTRAINT(X) typename
#define AL_REQUIRES(...)
#endif

#if defined(_MSC_VER)
#define AL_MSVC 1
#define AL_CLANG 0
#define AL_GCC 0
#elif defined(__GNUC__)
#define AL_GCC 1
#define AL_MSVC 0
#define AL_CLANG 0
#elif defined(__clang__)
#define AL_CLANG 1
#define AL_MSVC 0
#define AL_GCC 0
#else
#define AL_GCC 0
#define AL_MSVC 0
#define AL_CLANG 0
#endif

namespace al {

#define AL_NODISCARD [[nodiscard]]

#if AL_HAS_CONCEPTS
template <typename Container>
concept IterableContainer = requires(Container c) {
    std::begin(c);
    std::end(c);
};
#endif  // ^^^ AL_HAS_CONCEPTS

constexpr auto operator""_UZ(const unsigned long long value) -> size_t {
    return static_cast<size_t>(value);
}

namespace detail {

struct First {};
struct Second {};

template <class Ty1, class Ty2,
          bool DerivesFromFirst =
              std::is_empty<Ty1>::value and not std::is_final<Ty1>::value>
struct CompressedPair : private Ty1 {
    template <typename std::enable_if<std::is_constructible<Ty1>::value and
                                          std::is_constructible<Ty2>::value,
                                      int>::type = 0>
    CompressedPair() {}

    template <class First, class Second>
    AL_CONSTEXPR_CXX17 explicit CompressedPair(First&& first, Second&& second)
        : Ty1(std::forward<First>(first)),
          second_(std::forward<Second>(second)) {}

    template <class F>
    AL_CONSTEXPR_CXX17 explicit CompressedPair(First /* first */, F&& first)
        : Ty1(std::forward<F>(first)) {}

    template <class S>
    AL_CONSTEXPR_CXX17 explicit CompressedPair(Second /* second */, S&& second)
        : second_(std::forward<S>(second)) {}

    auto get_first() noexcept -> Ty1& { return *this; }
    auto get_first() const noexcept -> const Ty1& { return *this; }
    auto get_second() noexcept -> Ty2& { return second_; }
    auto get_second() const noexcept -> const Ty2& { return second_; }

    Ty2 second_;
};

template <class Ty1, class Ty2>
struct CompressedPair<Ty1, Ty2, false> {
    template <typename std::enable_if<std::is_constructible<Ty1>::value and
                                          std::is_constructible<Ty2>::value,
                                      int>::type = 0>
    CompressedPair() {}

    template <class First, class Second>
    AL_CONSTEXPR_CXX17 explicit CompressedPair(First&& first, Second&& second)
        : first_(std::forward<First>(first)),
          second_(std::forward<Second>(second)) {}

    template <class F>
    AL_CONSTEXPR_CXX17 explicit CompressedPair(First /* first */, F&& first)
        : first_(std::forward<F>(first)) {}

    template <class S>
    AL_CONSTEXPR_CXX17 explicit CompressedPair(Second /* second */, S&& second)
        : second_(std::forward<S>(second)) {}

    auto get_first() noexcept -> Ty1& { return first_; }
    auto get_first() const noexcept -> const Ty1& { return first_; }
    auto get_second() noexcept -> Ty2& { return second_; }
    auto get_second() const noexcept -> const Ty2& { return second_; }

    Ty1 first_;
    Ty2 second_;
};

template <class Iter>
using IterConcatenateType =
    typename std::iterator_traits<Iter>::iterator_category;

template <typename T>
struct IsIteratorHelper {
    template <typename U>
    static auto test(typename std::iterator_traits<U>::iterator_category*)
        -> std::true_type;

    template <typename U>
    static auto test(...) -> std::false_type;
};

template <typename T>
constexpr inline bool IsIteratorV =
    decltype(IsIteratorHelper<T>::template test<T>(nullptr))::value;

template <typename T>
struct IsIterator : std::bool_constant<IsIteratorV<T>> {};

#if AL_HAS_CONCEPTS
template <typename Type>
concept Iterator = IsIteratorV<Type>;
#endif  // ^^^ AL_HAS_CONCEPTS

template <class Iter, class WantedIter>
constexpr inline bool IsIteratorCategory =
    std::is_same<typename std::iterator_traits<Iter>::iterator_category,
                 WantedIter>::value;

template <class Iter>
constexpr inline bool IsRandomAccessIterator =
    IsIteratorCategory<Iter, std::random_access_iterator_tag>;

template <class AltyTraits, class Type, class Ally>
AL_CONSTEXPR_CXX20 auto destroy_in_place(Type* value, Ally&& ally) noexcept ->
    typename std::enable_if<
        std::is_array<typename std::remove_pointer<Type>::type>::value,
        void>::type {
    for (auto& element : *value) {
        destroy_in_place<AltyTraits>(std::addressof(element),
                                     std::forward<Ally>(ally));
    }
}

template <class AltyTraits, class Type, class Ally>
AL_CONSTEXPR_CXX20 auto destroy_in_place(Type* value, Ally&& ally) noexcept ->
    typename std::enable_if<
        !std::is_array<typename std::remove_pointer<Type>::type>::value,
        void>::type {
    AltyTraits::destroy(std::forward<Ally>(ally), value);
}

template <typename AltyTraits, typename It, typename Sentinel, class Ally>
AL_CONSTEXPR_CXX20 auto destroy_range(It first, Sentinel last,
                                      Ally&& ally) noexcept ->
    typename std::enable_if<
        !std::is_trivially_destructible<typename AltyTraits::value_type>::value,
        void>::type {
    for (; first != last; ++first) {
        destroy_in_place<AltyTraits>(std::addressof(*first),
                                     std::forward<Ally>(ally));
    }
}

template <typename AltyTraits, typename It, typename Sentinel, class Ally>
AL_CONSTEXPR_CXX20 auto destroy_range(It first, Sentinel last,
                                      Ally&& ally) noexcept ->
    typename std::enable_if<
        std::is_trivially_destructible<typename AltyTraits::value_type>::value,
        void>::type {}

template <class AltyTraits, class It, class Sentinel, class Ally>
AL_CONSTEXPR_CXX20 auto destruct_all_elements(It begin, Sentinel end,
                                              Ally&& ally) noexcept ->
    typename std::enable_if<
        !std::is_trivially_destructible<typename AltyTraits::value_type>::value,
        void>::type {
    destroy_range<AltyTraits>(begin, end, std::forward<Ally>(ally));
}

template <class AltyTraits, class It, class Sentinel, class Ally>
AL_CONSTEXPR_CXX20 auto destruct_all_elements(It begin, Sentinel end,
                                              Ally&& ally) noexcept ->
    typename std::enable_if<
        std::is_trivially_destructible<typename AltyTraits::value_type>::value,
        void>::type {}

}  // namespace detail

/// \deprecated
/// This is the difference in size. old capacity + this value is the new
/// prefered size
struct ArrayListDefaultGrowthDifference {
    constexpr auto operator()(const size_t old_capacity) -> size_t {
#if AL_CLANG || AL_GCC
        // return old_capacity;
        return old_capacity / 2;
#elif AL_MSVC
        return old_capacity / 2;
#else
        return old_capacity / 2;
#endif
    }
};

template <typename Type, typename Allocator = std::allocator<Type>>
AL_REQUIRES(std::is_object<Type>::value)
class ArrayList {
    static_assert(
        std::is_same<Type, typename Allocator::value_type>::value,
        "Requires allocator's type to match the type held by the ArrayList");
    static_assert(std::is_object<Type>::value,
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
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    // NOLINTEND

    static constexpr auto max_size() noexcept -> size_type {
        return static_cast<size_type>(-1) / sizeof(value_type);
    }

   private:
    constexpr auto calculate_growth(const size_type new_size) const noexcept
        -> size_type {
        const auto old_capacity = capacity();

        if (new_size > max_size() - old_capacity / 2) {
            return max_size();
        }
        const auto growth = old_capacity + (old_capacity / 2);

        if (growth < new_size) {
            return new_size;
        }
        return growth;
    }

    AL_NODISCARD constexpr auto get_allocator() noexcept -> allocator_type& {
        return compressed_.get_first();
    }

   public:
    constexpr ArrayList() noexcept = default;

    AL_CONSTEXPR_CXX20 explicit ArrayList(
        const size_type capacity,
        const allocator_type& alloc = allocator_type())
        : compressed_(
              alloc, Payload{AltyTraits::allocate(get_allocator(), capacity)}) {
        auto& p = payload();
        p.end = p.data + capacity;
        p.current = p.data;
    }

// NOLINTBEGIN
#if AL_HAS_CONCEPTS
    template <typename Iter>
        requires(detail::IsRandomAccessIterator<Iter>)
#else
    template <typename Iter,
              typename std::enable_if<detail::IsRandomAccessIterator<Iter>,
                                      int>::type = 0>
#endif
    AL_CONSTEXPR_CXX20 ArrayList(Iter first, Iter last,
                                 const allocator_type& alloc = allocator_type())
        : compressed_(detail::First{}, alloc) {
        const auto length = static_cast<size_t>(std::distance(first, last));
        auto& p = payload();

        p.data = AltyTraits::allocate(get_allocator(), length);
        p.end = p.data + length;
        p.current = p.data + length;

        std::uninitialized_copy(first, last, p.data);
    }

#if AL_HAS_CONCEPTS
    template <typename Iter>
        requires(!detail::IsRandomAccessIterator<Iter>)
#else
    template <typename Iter,
              typename std::enable_if<!detail::IsRandomAccessIterator<Iter>,
                                      int>::type = 0>
#endif
    AL_CONSTEXPR_CXX20 ArrayList(Iter first, Iter last,
                                 const allocator_type& alloc = allocator_type())
        : compressed_(detail::First{}, alloc) {
        for (; first != last; ++first) {
            push_back(*first);
        }
    }

    AL_CONSTEXPR_CXX20 ArrayList(std::initializer_list<Type> list,
                                 const allocator_type& alloc = allocator_type())
        : ArrayList(list.begin(), list.end(), alloc) {}

    template <AL_CONSTRAINT(IterableContainer) Container>
    AL_CONSTEXPR_CXX20 explicit ArrayList(
        const Container& container,
        const allocator_type& alloc = allocator_type())
        : ArrayList(container.begin(), container.end(), alloc) {
        const auto container_size = std::size(container);
        reserve(container_size);
        auto& p = payload();
        std::uninitialized_copy(std::begin(container), std::end(container),
                                p.data);
        p.current = p.data + container_size;
    }

    AL_CONSTEXPR_CXX20 ArrayList(const ArrayList& other,
                                 const allocator_type& alloc = allocator_type())
        : ArrayList(other.begin(), other.end(), alloc) {}

    constexpr ArrayList(ArrayList&& other) noexcept
        : compressed_(std::exchange(other.compressed_, Compressed())) {}

    AL_CONSTEXPR_CXX20 auto operator=(const ArrayList& other) -> ArrayList& {
        if (this != std::addressof(other)) {
            copy_safe(other);
        }
        return *this;
    }

    constexpr auto operator=(ArrayList&& other) noexcept -> ArrayList& {
        destruct_all_elements();
        deallocate_ptr();

        compressed_ = std::exchange(other.compressed_, Compressed());

        return *this;
    }

    AL_CONSTEXPR_CXX20 ~ArrayList() {
        destruct_all_elements();
        deallocate_ptr();
    }

    AL_NODISCARD constexpr auto empty() const noexcept -> bool {
        auto& p = payload();
        return p.data == p.current;
    }

#if AL_HAS_CONCEPTS
    template <typename Iter>
        requires(detail::IsIteratorV<Iter>)
#else
    template <typename Iter,
              typename std::enable_if<detail::IsIteratorV<Iter>, int>::type = 0>
#endif
    constexpr auto push_back(Iter first, Iter last) -> void {
        const auto length = std::distance(first, last);
        ensure_size_for_elements(length);

        auto& p = payload();

        std::uninitialized_copy(first, last, p.current);
        p.current += length;
    }

    AL_CONSTEXPR_CXX20 auto push_back(const Type& value) -> void {
        ensure_size_for_elements(1_UZ);
        raw_push_back(value);
    }

    AL_CONSTEXPR_CXX20 auto push_back(Type&& value) -> void {
        ensure_size_for_elements(1_UZ);
        raw_push_back(std::move(value));
    }

    template <typename... Args>
    AL_CONSTEXPR_CXX20 auto emplace_back(Args&&... args) -> value_type& {
        ensure_size_for_elements(1_UZ);
        return raw_emplace_back(std::forward<Args>(args)...);
    }

    AL_CONSTEXPR_CXX20 void pop_back() {
        ensure_not_empty();
        // destroy it!
        auto& p = payload();
        destroy_in_place(p.current);
        --p.current;
    }

    AL_NODISCARD constexpr auto size() const noexcept -> size_type {
        auto& p = payload();
        return p.current - p.data;
    }

#if !AL_HAS_CONCEPTS
    template <typename std::enable_if<std::is_constructible<value_type>::value,
                                      int>::type = 0>
#endif
    AL_CONSTEXPR_CXX20 void resize(const size_type new_size)
#if AL_HAS_CONCEPTS
        requires(std::is_constructible<value_type>::value)
#endif
    {
        const auto len = size();
        auto& p = payload();

        if (new_size < len) {
            detail::destroy_range<AltyTraits>(p.data + new_size, p.data + len,
                                              get_allocator());
        }

        if (capacity() < new_size) {
            reserve(new_size);
        }
        auto* tmp = p.current;
        p.current = p.data + new_size;

        std::uninitialized_value_construct_n(tmp, p.current - tmp);
    }

    AL_CONSTEXPR_CXX20 void reserve(const size_type new_capacity) {
        const auto cap = capacity();
        if (cap >= new_capacity) {
            return;
        }

        const auto len = size();

        auto& p = payload();

        const auto old_ptr = p.data;
        raw_reserve(new_capacity);
        if (old_ptr) {
            if (cap > 0) {
                if (len > 0) {
                    std::uninitialized_move_n(old_ptr, len, p.data);
                }
                destroy_range(old_ptr, old_ptr + len);
                deallocate_target_ptr(old_ptr, cap);
            }
        }
    }

    AL_NODISCARD constexpr auto operator[](const size_type index) noexcept
        -> reference {
        return payload().data[index];
    }

    AL_NODISCARD constexpr auto operator[](const size_type index) const noexcept
        -> const_reference {
        return payload().data[index];
    }

    AL_NODISCARD constexpr auto at(const size_type index) -> reference {
        ensure_in_range(index);
        return payload().data[index];
    }

    AL_NODISCARD constexpr auto at(const size_type index) const
        -> const_reference {
        ensure_in_range(index);
        return payload().data[index];
    }

    AL_NODISCARD constexpr auto capacity() const noexcept -> size_type {
        auto& p = payload();
        return p.end - p.data;
    }

    AL_NODISCARD constexpr auto data() noexcept -> pointer {
        return payload().data;
    }

    AL_NODISCARD constexpr auto data() const noexcept -> const_pointer {
        return payload().data;
    }

    AL_NODISCARD constexpr auto front() -> reference {
        ensure_not_empty();
        return *payload().data;
    }

    AL_NODISCARD constexpr auto front() const -> const_reference {
        ensure_not_empty();
        return *payload().data;
    }

    AL_NODISCARD constexpr auto back() -> reference {
        ensure_not_empty();
        return *(payload().current - 1);
    }

    AL_NODISCARD constexpr auto back() const -> const_reference {
        ensure_not_empty();
        return *(payload().current - 1);
    }

    AL_NODISCARD constexpr auto begin() noexcept -> iterator {
        return iterator(payload().data);
    }

    AL_NODISCARD constexpr auto end() noexcept -> iterator {
        return iterator(payload().current);
    }

    AL_NODISCARD constexpr auto begin() const noexcept -> const_iterator {
        return const_iterator(payload().data);
    }

    AL_NODISCARD constexpr auto end() const noexcept -> const_iterator {
        return const_iterator(payload().current);
    }

    AL_NODISCARD constexpr auto cbegin() const noexcept -> const_iterator {
        return const_iterator(payload().data);
    }

    AL_NODISCARD constexpr auto cend() const noexcept -> const_iterator {
        return const_iterator(payload().current);
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

    AL_NODISCARD constexpr auto rend() const noexcept
        -> const_reverse_iterator {
        return const_reverse_iterator(begin());
    }

    AL_NODISCARD constexpr auto crbegin() const noexcept
        -> const_reverse_iterator {
        return rbegin();
    }

    AL_NODISCARD constexpr auto crend() const noexcept
        -> const_reverse_iterator {
        return rend();
    }

    AL_CONSTEXPR_CXX20 auto clear() noexcept -> void {
        destruct_all_elements();
        auto& p = payload();
        p.current = p.data;
    }

    AL_CONSTEXPR_CXX20 auto erase(const_iterator position) -> iterator {
        const auto index = static_cast<size_type>(position - begin());
        erase(index);
        return begin() + index;
    }

    AL_CONSTEXPR_CXX20 auto erase(const size_type index) -> iterator {
        if (index >= size() or size() == 0) {
            throw std::out_of_range("Index out of range");
        }
        const auto length = size();
        auto& p = payload();
        std::move(p.data + index + 1, p.data + length + 1, p.data + index);
        --p.current;
        return p.data + index;
    }

    constexpr explicit operator bool() const noexcept { return !empty(); }

   private:
    friend constexpr auto operator==(const ArrayList& self,
                                     const ArrayList& that) noexcept -> bool {
        if (self.size() != that.size()) {
            return false;
        }
        return std::equal(self.begin(), self.end(), that.begin());
    }

    friend constexpr auto operator<(const ArrayList& self,
                                    const ArrayList& that) noexcept -> bool {
        auto it = self.begin();
        auto it2 = that.begin();
        for (; it != self.end() && it2 != that.end(); ++it, ++it2) {
            if (!(*it < *it2)) {
                return false;
            }
        }
        if (it != self.end()) {
            return false;
        }
        if (it2 == that.end()) {
            return false;
        }
        return true;
    }

    friend constexpr auto operator>(const ArrayList& self,
                                    const ArrayList& that) noexcept -> bool {
        return that < self;
    }

    friend constexpr auto operator<=(const ArrayList& self,
                                     const ArrayList& that) noexcept -> bool {
        return !(self > that);
    }

    friend constexpr auto operator>=(const ArrayList& self,
                                     const ArrayList& that) noexcept -> bool {
        return !(self < that);
    }

    AL_CONSTEXPR_CXX20 void raw_reserve(const size_type capacity) {
        const auto length = size();
        payload().data = AltyTraits::allocate(get_allocator(), capacity);
        payload().current = payload().data + length;
        payload().end = payload().data + capacity;
    }

    AL_CONSTEXPR_CXX20 void copy_safe(const ArrayList& other) {
        destruct_all_elements();
        deallocate_ptr();

        reserve(other.size());
        copy_unsafe(other);
    }

    AL_CONSTEXPR_CXX20 void copy_unsafe(const ArrayList& other) {
        const auto length{other.size()};
        payload().current = payload().data + length;
        std::uninitialized_copy_n(other.payload().data, length, payload().data);
    }

    constexpr void ensure_in_range(const size_type index) const {
        if (index >= size() or payload().data == nullptr) {
            throw std::out_of_range("Index out of range");
        }
    }

    constexpr void ensure_not_empty() const {
        if (size() <= 0_UZ or payload().data == nullptr) {
            throw std::out_of_range("ArrayList is empty");
        }
    }

    AL_CONSTEXPR_CXX20 void ensure_size_for_elements(const size_type elements) {
        if (!(size() + elements <= capacity())) {
            auto new_capacity = calculate_growth(capacity() + elements);
            reserve(new_capacity);
        }
    }

    AL_CONSTEXPR_CXX20 void grow_capacity() {
        return ensure_size_for_elements(1);
    }

    template <typename... Args>
    AL_CONSTEXPR_CXX20 auto raw_emplace_back(Args&&... args) -> value_type& {
        return emplace_at_back(std::forward<Args>(args)...);
    }

    AL_CONSTEXPR_CXX20 auto raw_push_back(const Type& value) -> void {
        push_at_back(value);
    }

    AL_CONSTEXPR_CXX20 auto raw_push_back(Type&& value) -> void {
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

    AL_CONSTEXPR_CXX20 auto raw_push_into(value_type* const my_ptr,
                                          Type&& value) -> void {
        AltyTraits::construct(get_allocator(), my_ptr, std::move(value));
    }

    template <typename... Args>
    AL_CONSTEXPR_CXX20 auto emplace_at_back(Args&&... args) -> value_type& {
        return raw_emplace_into(payload().current++,
                                std::forward<Args>(args)...);
    }

    AL_CONSTEXPR_CXX20 auto push_at_back(const Type& value) -> void {
        raw_push_into(payload().current++, value);
    }

    AL_CONSTEXPR_CXX20 auto push_at_back(Type&& value) -> void {
        raw_push_into(payload().current++, std::move(value));
    }

    AL_CONSTEXPR_CXX20 auto destroy_in_place(Type* value) noexcept -> void {
        AltyTraits::destroy(get_allocator(), value);
        detail::destroy_in_place<AltyTraits>(value, get_allocator());
    }

    template <typename It, typename Sentinel>
    AL_CONSTEXPR_CXX20 auto destroy_range(It first, Sentinel last) noexcept
        -> void {
        detail::destroy_range<AltyTraits>(first, last, get_allocator());
    }

    AL_CONSTEXPR_CXX20 auto destruct_all_elements() noexcept -> void {
        detail::destruct_all_elements<AltyTraits>(begin(), end(),
                                                  get_allocator());
    }

    AL_CONSTEXPR_CXX20 auto deallocate_target_ptr(value_type* const ptr,
                                                  const size_type length)
        -> void {
        if (ptr) {
            AltyTraits::deallocate(get_allocator(), ptr, length);
        }
    }

    AL_CONSTEXPR_CXX20 auto deallocate_ptr() -> void {
        deallocate_target_ptr(data(), capacity());
    }

    constexpr auto swap_with_other(ArrayList& other) -> void {
        using std::swap;
        swap(compressed_, other.compressed_);
    }

    struct Payload {
        constexpr Payload() = default;

        constexpr explicit Payload(pointer data, pointer end, pointer current)
            : data(data), end(end), current(current) {}

        constexpr explicit Payload(pointer data) : Payload(data, data, data) {}

        constexpr explicit Payload(pointer data, pointer end)
            : Payload(data, end, data) {}

        pointer data = nullptr;
        pointer end = nullptr;
        pointer current = nullptr;
    };

    constexpr auto payload() noexcept -> Payload& {
        return compressed_.get_second();
    }

    constexpr auto payload() const noexcept -> const Payload& {
        return compressed_.get_second();
    }

    using Compressed = detail::CompressedPair<allocator_type, Payload>;

    Compressed compressed_;
};

}  // namespace al

#endif  // ARRAY_LIST_HPP
