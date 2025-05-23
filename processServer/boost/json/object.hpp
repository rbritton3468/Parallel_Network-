//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_OBJECT_HPP
#define BOOST_JSON_OBJECT_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/detail/object.hpp>
#include <boost/json/detail/value.hpp>
#include <boost/json/kind.hpp>
#include <boost/json/pilfer.hpp>
#include <boost/system/result.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/string_view.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

class value;
class value_ref;
class key_value_pair;

/** A dynamically sized associative container of JSON key/value pairs.

    This is an associative container whose elements
    are key/value pairs with unique keys.

    The elements are stored contiguously; iterators are
    ordinary pointers, allowing random access pointer
    arithmetic for retrieving elements.
    In addition, the container maintains an internal
    index to speed up find operations, reducing the
    average complexity for most lookups and insertions.

    Reallocations are usually costly operations in terms of
    performance, as elements are copied and the internal
    index must be rebuilt. The @ref reserve function can
    be used to eliminate reallocations if the number of
    elements is known beforehand.

    @par Allocators

    All elements stored in the container, and their
    children if any, will use the same memory resource that
    was used to construct the container.

    @par Thread Safety

    Non-const member functions may not be called
    concurrently with any other member functions.

    @par Satisfies
        <a href="https://en.cppreference.com/w/cpp/named_req/ContiguousContainer"><em>ContiguousContainer</em></a>,
        <a href="https://en.cppreference.com/w/cpp/named_req/ReversibleContainer"><em>ReversibleContainer</em></a>, and
        <a href="https://en.cppreference.com/w/cpp/named_req/SequenceContainer"><em>SequenceContainer</em></a>.
*/
class object
{
    struct table;
    class revert_construct;
    class revert_insert;
    friend class value;
    friend class object_test;
    using access = detail::access;
    using index_t = std::uint32_t;
    static index_t constexpr null_index_ =
        std::uint32_t(-1);

    storage_ptr sp_;            // must come first
    kind k_ = kind::object;     // must come second
    table* t_;

    BOOST_JSON_DECL
    static table empty_;

    template<class T>
    using is_inputit = typename std::enable_if<
        std::is_constructible<key_value_pair,
        typename std::iterator_traits<T>::reference
            >::value>::type;

    BOOST_JSON_DECL
    explicit
    object(detail::unchecked_object&& uo);

public:
    /// Associated [Allocator](https://en.cppreference.com/w/cpp/named_req/Allocator)
    using allocator_type = container::pmr::polymorphic_allocator<value>;

    /** The type of keys.

        The function @ref string::max_size returns the
        maximum allowed size of strings used as keys.
    */
    using key_type = string_view;

    /// The type of mapped values
    using mapped_type = value;

    /// The element type
    using value_type = key_value_pair;

    /// The type used to represent unsigned integers
    using size_type = std::size_t;

    /// The type used to represent signed integers
    using difference_type = std::ptrdiff_t;

    /// A reference to an element
    using reference = value_type&;

    /// A const reference to an element
    using const_reference = value_type const&;

    /// A pointer to an element
    using pointer = value_type*;

    /// A const pointer to an element
    using const_pointer = value_type const*;

    /// A random access iterator to an element
    using iterator = value_type*;

    /// A const random access iterator to an element
    using const_iterator = value_type const*;

    /// A reverse random access iterator to an element
    using reverse_iterator =
        std::reverse_iterator<iterator>;

    /// A const reverse random access iterator to an element
    using const_reverse_iterator =
        std::reverse_iterator<const_iterator>;

    //------------------------------------------------------

    /** Destructor.

        The destructor for each element is called if needed, any used memory is
        deallocated, and shared ownership of the
        `boost::container::pmr::memory_resource` is released.

        @par Complexity
        Constant, or linear in @ref size().

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    ~object() noexcept;

    //------------------------------------------------------

    /** Default constructor.

        The constructed object is empty with zero
        capacity, using the [default memory resource].

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        [default memory resource]: json/allocators/storage_ptr.html#json.allocators.storage_ptr.default_memory_resource
    */
    object() noexcept
        : t_(&empty_)
    {
    }

    /** Constructor.

        The constructed object is empty with zero
        capacity, using the specified memory resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param sp A pointer to the `boost::container::pmr::memory_resource` to
        use. The container will acquire shared ownership of the memory
        resource.
    */
    explicit
    object(storage_ptr sp) noexcept
        : sp_(std::move(sp))
        , t_(&empty_)
    {
    }

    /** Constructor.

        The constructed object is empty with capacity
        equal to the specified minimum capacity,
        using the specified memory resource.

        @par Complexity
        Constant.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param min_capacity The minimum number
        of elements for which capacity is guaranteed
        without a subsequent reallocation.

        @param sp A pointer to the `boost::container::pmr::memory_resource` to
        use. The container will acquire shared ownership of the memory
        resource.
    */
    BOOST_JSON_DECL
    object(
        std::size_t min_capacity,
        storage_ptr sp = {});

    /** Constructor.

        The object is constructed with the elements
        in the range `{first, last)`, preserving order,
        using the specified memory resource.
        If there are elements with duplicate keys; that
        is, if multiple elements in the range have keys
        that compare equal, only the first equivalent
        element will be inserted.

        @par Constraints
        @code
        std::is_constructible_v<
            key_value_pair,
            std::iterator_traits<InputIt>::reference>
        @endcode

        @par Complexity
        Linear in `std::distance(first, last)`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param first An input iterator pointing to the
        first element to insert, or pointing to the end
        of the range.

        @param last An input iterator pointing to the end
        of the range.

        @param min_capacity The minimum number
        of elements for which capacity is guaranteed
        without a subsequent reallocation.
        Upon construction, @ref capacity() will be greater
        than or equal to this number.

        @param sp A pointer to the `boost::container::pmr::memory_resource` to
        use. The container will acquire shared ownership of the memory
        resource.

        @tparam InputIt a type satisfying the requirements
        of __InputIterator__.
    */
    template<
        class InputIt
    #ifndef BOOST_JSON_DOCS
        ,class = is_inputit<InputIt>
    #endif
    >
    object(
        InputIt first,
        InputIt last,
        std::size_t min_capacity = 0,
        storage_ptr sp = {})
        : sp_(std::move(sp))
        , t_(&empty_)
    {
        construct(
            first, last,
            min_capacity,
            typename std::iterator_traits<
                InputIt>::iterator_category{});
    }

    /** Move constructor.

        The object is constructed by acquiring ownership of
        the contents of `other` and shared ownership
        of `other`'s memory resource.

        @note

        After construction, the moved-from object behaves
        as if newly constructed with its current memory resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param other The object to move.
    */
    BOOST_JSON_DECL
    object(object&& other) noexcept;

    /** Move constructor.

        The object is constructed with the contents of
        `other` by move semantics, using the specified
        memory resource:

        @li If `*other.storage() == *sp`, ownership of
        the underlying memory is transferred in constant
        time, with no possibility of exceptions.
        After construction, the moved-from object behaves
        as if newly constructed with its current storage
        pointer.

        @li If `*other.storage() != *sp`, an
        element-wise copy is performed, which may throw.
        In this case, the moved-from object is not
        changed.

        @par Complexity
        Constant or linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The object to move.

        @param sp A pointer to the `boost::container::pmr::memory_resource` to
        use. The container will acquire shared ownership of the memory
        resource.
    */
    BOOST_JSON_DECL
    object(
        object&& other,
        storage_ptr sp);

    /** Pilfer constructor.

        The object is constructed by acquiring ownership
        of the contents of `other` using pilfer semantics.
        This is more efficient than move construction, when
        it is known that the moved-from object will be
        immediately destroyed afterwards.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param other The value to pilfer. After pilfer
        construction, `other` is not in a usable state
        and may only be destroyed.

        @see @ref pilfer,
            <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0308r0.html">
                Valueless Variants Considered Harmful</a>
    */
    object(pilfered<object> other) noexcept
        : sp_(std::move(other.get().sp_))
        , t_(detail::exchange(
            other.get().t_, &empty_))
    {
    }

    /** Copy constructor.

        The object is constructed with a copy of the
        contents of `other`, using `other`'s memory resource.

        @par Complexity
        Linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The object to copy.
    */
    object(
        object const& other)
        : object(other, other.sp_)
    {
    }

    /** Copy constructor.

        The object is constructed with a copy of the
        contents of `other`, using the specified memory resource.

        @par Complexity
        Linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The object to copy.

        @param sp A pointer to the `boost::container::pmr::memory_resource` to
        use. The container will acquire shared ownership of the memory
        resource.
    */
    BOOST_JSON_DECL
    object(
        object const& other,
        storage_ptr sp);

    /** Construct from initializer-list.

        The object is constructed with a copy of the values
        in the initializer-list in order, using the
        specified memory resource.
        If there are elements with duplicate keys; that
        is, if multiple elements in the range have keys
        that compare equal, only the first equivalent
        element will be inserted.

        @par Complexity
        Linear in `init.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param init The initializer list to insert.

        @param sp A pointer to the `boost::container::pmr::memory_resource` to
        use. The container will acquire shared ownership of the memory
        resource.
    */
    object(
        std::initializer_list<
            std::pair<string_view, value_ref>> init,
        storage_ptr sp = {})
        : object(init, 0, std::move(sp))
    {
    }

    /** Construct from initializer-list.

        Storage for at least `min_capacity` elements is
        reserved, and then
        the object is constructed with a copy of the values
        in the initializer-list in order, using the
        specified memory resource.
        If there are elements with duplicate keys; that
        is, if multiple elements in the range have keys
        that compare equal, only the first equivalent
        element will be inserted.

        @par Complexity
        Linear in `init.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param init The initializer list to insert.

        @param min_capacity The minimum number
        of elements for which capacity is guaranteed
        without a subsequent reallocation.
        Upon construction, @ref capacity() will be greater
        than or equal to this number.

        @param sp A pointer to the `boost::container::pmr::memory_resource` to
        use. The container will acquire shared ownership of the memory
        resource.
    */
    BOOST_JSON_DECL
    object(
        std::initializer_list<
            std::pair<string_view, value_ref>> init,
        std::size_t min_capacity,
        storage_ptr sp = {});

    //------------------------------------------------------
    //
    // Assignment
    //
    //------------------------------------------------------

    /** Copy assignment.

        The contents of the object are replaced with an
        element-wise copy of `other`.

        @par Complexity
        Linear in @ref size() plus `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The object to copy.
    */
    BOOST_JSON_DECL
    object&
    operator=(object const& other);

    /** Move assignment.

        The contents of the object are replaced with the
        contents of `other` using move semantics:

        @li If `*other.storage() == *sp`, ownership of
        the underlying memory is transferred in constant
        time, with no possibility of exceptions.
        After assignment, the moved-from object behaves
        as if newly constructed with its current storage
        pointer.

        @li If `*other.storage() != *sp`, an
        element-wise copy is performed, which may throw.
        In this case, the moved-from object is not
        changed.

        @par Complexity
        Constant or linear in @ref size() plus `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The object to move.
    */
    BOOST_JSON_DECL
    object&
    operator=(object&& other);

    /** Assignment.

        Replaces the contents with the contents of an
        initializer list.

        @par Complexity
        Linear in @ref size() plus
        average case linear in `init.size()`,
        worst case quadratic in `init.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param init The initializer list to copy.
    */
    BOOST_JSON_DECL
    object&
    operator=(std::initializer_list<
        std::pair<string_view, value_ref>> init);

    //------------------------------------------------------

    /** Return the associated memory resource.

        This function returns the `boost::container::pmr::memory_resource` used
        by the container.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    storage_ptr const&
    storage() const noexcept
    {
        return sp_;
    }

    /** Return the associated allocator.

        This function returns an instance of @ref allocator_type constructed
        from the associated `boost::container::pmr::memory_resource`.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    allocator_type
    get_allocator() const noexcept
    {
        return sp_.get();
    }

    //------------------------------------------------------
    //
    // Iterators
    //
    //------------------------------------------------------

    /** Return an iterator to the first element.

        If the container is empty, @ref end() is returned.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    iterator
    begin() noexcept;

    /** Return a const iterator to the first element.

        If the container is empty, @ref end() is returned.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_iterator
    begin() const noexcept;

    /** Return a const iterator to the first element.

        If the container is empty, @ref cend() is returned.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_iterator
    cbegin() const noexcept;

    /** Return an iterator to the element following the last element.

        The element acts as a placeholder; attempting
        to access it results in undefined behavior.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    iterator
    end() noexcept;

    /** Return a const iterator to the element following the last element.

        The element acts as a placeholder; attempting
        to access it results in undefined behavior.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_iterator
    end() const noexcept;

    /** Return a const iterator to the element following the last element.

        The element acts as a placeholder; attempting
        to access it results in undefined behavior.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_iterator
    cend() const noexcept;

    /** Return a reverse iterator to the first element of the reversed container.

        The pointed-to element corresponds to the
        last element of the non-reversed container.
        If the container is empty, @ref rend() is returned.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    reverse_iterator
    rbegin() noexcept;

    /** Return a const reverse iterator to the first element of the reversed container.

        The pointed-to element corresponds to the
        last element of the non-reversed container.
        If the container is empty, @ref rend() is returned.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_reverse_iterator
    rbegin() const noexcept;

    /** Return a const reverse iterator to the first element of the reversed container.

        The pointed-to element corresponds to the
        last element of the non-reversed container.
        If the container is empty, @ref crend() is returned.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_reverse_iterator
    crbegin() const noexcept;

    /** Return a reverse iterator to the element following the last element of the reversed container.

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting
        to access it results in undefined behavior.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    reverse_iterator
    rend() noexcept;

    /** Return a const reverse iterator to the element following the last element of the reversed container.

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting
        to access it results in undefined behavior.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_reverse_iterator
    rend() const noexcept;

    /** Return a const reverse iterator to the element following the last element of the reversed container.

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting
        to access it results in undefined behavior.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_reverse_iterator
    crend() const noexcept;

    //------------------------------------------------------
    //
    // Capacity
    //
    //------------------------------------------------------

    /** Return whether there are no elements.

        Returns `true` if there are no elements in
        the container, i.e. @ref size() returns 0.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    bool
    empty() const noexcept;

    /** Return the number of elements.

        This returns the number of elements in the container.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    std::size_t
    size() const noexcept;

    /** Return the maximum number of elements any object can hold

        The maximum is an implementation-defined number dependent
        on system or library implementation. This value is a
        theoretical limit; at runtime, the actual maximum size
        may be less due to resource limits.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    static
    constexpr
    std::size_t
    max_size() noexcept;

    /** Return the number of elements that can be held in currently allocated memory

        This number may be larger than the value returned
        by @ref size().

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    std::size_t
    capacity() const noexcept;

    /** Increase the capacity to at least a certain amount.

        This increases the @ref capacity() to a value
        that is greater than or equal to `new_capacity`.
        If `new_capacity > capacity()`, new memory is
        allocated. Otherwise, the call has no effect.
        The number of elements and therefore the
        @ref size() of the container is not changed.

        If new memory is allocated, all iterators
        including any past-the-end iterators, and all
        references to the elements are invalidated.
        Otherwise, no iterators or references are
        invalidated.

        @par Complexity
        Constant or average case linear in
        @ref size(), worst case quadratic.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param new_capacity The new minimum capacity.

        @throw `boost::system::system_error`  `new_capacity > max_size()`.
    */
    inline
    void
    reserve(std::size_t new_capacity);

    //------------------------------------------------------
    //
    // Modifiers
    //
    //------------------------------------------------------

    /** Erase all elements.

        Erases all elements from the container without
        changing the capacity.
        After this call, @ref size() returns zero.
        All references, pointers, and iterators are
        invalidated.

        @par Complexity
        Linear in @ref size().

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    void
    clear() noexcept;

    /** Insert elements.

        Inserts `p` if `this->contains(value_type(p).key())` is `false`.
        @ref value_type must be constructible from `p`.

        If the insertion occurs and results in a rehashing
        of the container, all iterators and references are invalidated.
        Otherwise, they are not affected.
        Rehashing occurs only if the new number of elements
        is greater than @ref capacity().

        @par Constraints
        @code
        std::is_constructible_v<value_type, P>
        @endcode

        @par Complexity
        Average case amortized constant,
        worst case linear in @ref size().

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param p The value to insert.

        @throw `boost::system::system_error` key is too long.
        @throw `boost::system::system_error` @ref size() >= max_size().

        @return A pair where `first` is an iterator
        to the existing or inserted element, and `second`
        is `true` if the insertion took place or `false` otherwise.
    */
    template<class P
#ifndef BOOST_JSON_DOCS
        ,class = typename std::enable_if<
            std::is_constructible<key_value_pair,
                P, storage_ptr>::value>::type
#endif
    >
    std::pair<iterator, bool>
    insert(P&& p);

    /** Insert elements.

        The elements in the range `[first, last)` are inserted one at a time,
        in order. Any element with key that is a duplicate of a key already
        present in container will be skipped. This also means, that if there
        are two keys within the range that are equal to each other, only the
        first will be inserted.

        @n

        Insertion may result in rehashing of the container. In that case all
        iterators and references are invalidated. Otherwise, they are not
        affected.

        @par Precondition
        `first` and `last` are not iterators into `*this`.
        `first` and `last` form a valid range.

        @par Constraints
        @code
        std::is_constructible_v<value_type, std::iterator_traits<InputIt>::reference>
        @endcode

        @par Complexity
        Linear in `std::distance(first, last)`.

        @par Exception Safety
        Strong guarantee for forward iterators, basic guarantee for input
        iterators.
        Calls to `memory_resource::allocate` may throw.

        @param first An input iterator pointing to the first
        element to insert, or pointing to the end of the range.

        @param last An input iterator pointing to the end
        of the range.

        @tparam InputIt a type satisfying the requirements
        of __InputIterator__.
    */
    template<
        class InputIt
    #ifndef BOOST_JSON_DOCS
        ,class = is_inputit<InputIt>
    #endif
    >
    void
    insert(InputIt first, InputIt last)
    {
        insert(first, last, typename
            std::iterator_traits<InputIt
                >::iterator_category{});
    }

    /** Insert elements.

        The elements in the initializer list are inserted one at a time, in
        order. Any element with key that is a duplicate of a key already
        present in container will be skipped. This also means, that if there
        are two keys within the initializer list that are equal to each other,
        only the first will be inserted.

        Insertion may result in rehashing of the container. In that case all
        iterators and references are invalidated. Otherwise, they are not
        affected.

        @par Complexity
        Linear in `init.size()`.

        @par Exception Safety
        Basic guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param init The initializer list to insert
    */
    BOOST_JSON_DECL
    void
    insert(std::initializer_list<
        std::pair<string_view, value_ref>> init);

    /** Insert an element or assign to the current element if the key already exists.

        If the key equivalent to `key` already exists in the
        container, assigns `std::forward<M>(m)` to the
        `mapped_type` corresponding to the key. Otherwise,
        inserts the new value at the end as if by insert,
        constructing it from `value_type(key, std::forward<M>(m))`.

        If the insertion occurs and results in a rehashing of the container,
        all iterators and references are invalidated. Otherwise, they are not
        affected. Rehashing occurs only if the new number of elements is
        greater than @ref capacity().

        @par Complexity
        Amortized constant on average, worst case linear in @ref size().

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @return A `std::pair` where `first` is an iterator
        to the existing or inserted element, and `second`
        is `true` if the insertion took place or `false` if
        the assignment took place.

        @param key The key used for lookup and insertion

        @param m The value to insert or assign

        @throw `boost::system::system_error` if key is too long.
    */
    template<class M>
    std::pair<iterator, bool>
    insert_or_assign(
        string_view key, M&& m);

    /** Construct an element in-place.

        Inserts a new element into the container constructed
        in-place with the given argument if there is no
        element with the `key` in the container.

        If the insertion occurs and results in a rehashing of the container,
        all iterators and references are invalidated. Otherwise, they are not
        affected. Rehashing occurs only if the new number of elements is
        greater than @ref capacity().

        @par Complexity
        Amortized constant on average, worst case linear in @ref size().

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @return A `std::pair` where `first` is an iterator
        to the existing or inserted element, and `second`
        is `true` if the insertion took place or `false` otherwise.

        @param key The key used for lookup and insertion

        @param arg The argument used to construct the value.
        This will be passed as `std::forward<Arg>(arg)` to
        the @ref value constructor.

        @throw `boost::system::system_error` if key is too long.
    */
    template<class Arg>
    std::pair<iterator, bool>
    emplace(string_view key, Arg&& arg);

    /** Erase an element

        Remove the element pointed to by `pos`, which must
        be valid and dereferenceable.
        References and iterators to the erased element are
        invalidated. Other iterators and references are not
        invalidated.

        @note

        The @ref end() iterator (which is valid but cannot be
        dereferenced) cannot be used as a value for `pos`.

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @return An iterator following the removed element.

        @param pos An iterator pointing to the element to be
        removed.
    */
    BOOST_JSON_DECL
    iterator
    erase(const_iterator pos) noexcept;

    /** Erase an element

        Remove the element which matches `key`, if it exists.
        References and iterators to the erased element are
        invalidated. Other iterators and references are not
        invalidated.

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @return The number of elements removed, which will
        be either 0 or 1.

        @param key The key to match.
    */
    BOOST_JSON_DECL
    std::size_t
    erase(string_view key) noexcept;

    /** Erase an element preserving order

        Remove the element pointed to by `pos`, which must be valid and
        dereferenceable. References and iterators from `pos` to @ref end(),
        both included, are invalidated. Other iterators and references are not
        invalidated. The relative order of remaining elements is preserved.

        @note
        The @ref end() iterator (which is valid but cannot be dereferenced)
        cannot be used as a value for `pos`.

        @par Complexity
        Linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @return An iterator following the removed element.

        @param pos An iterator pointing to the element to be
        removed.
    */
    BOOST_JSON_DECL
    iterator
    stable_erase(const_iterator pos) noexcept;

    /** Erase an element preserving order

        Remove the element which matches `key`, if it exists.
        All references and iterators are invalidated.
        The relative order of remaining elements is preserved.

        @par Complexity
        Linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @return The number of elements removed, which will
        be either 0 or 1.

        @param key The key to match.
    */
    BOOST_JSON_DECL
    std::size_t
    stable_erase(string_view key) noexcept;

    /** Swap two objects.

        Exchanges the contents of this object with another object. Ownership of
        the respective `boost::container::pmr::memory_resource` objects is not
        transferred.

        @li If `*other.storage() == *this->storage()`,
        ownership of the underlying memory is swapped in
        constant time, with no possibility of exceptions.
        All iterators and references remain valid.

        @li If `*other.storage() != *this->storage()`,
        the contents are logically swapped by making copies,
        which can throw. In this case all iterators and
        references are invalidated.

        @par Complexity
        Constant or linear in @ref size() plus `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The object to swap with.
        If `this == &other`, this function call has no effect.
    */
    BOOST_JSON_DECL
    void
    swap(object& other);

    /** Swap two objects.

        Exchanges the contents of the object `lhs` with another object `rhs`.
        Ownership of the respective `boost::container::pmr::memory_resource`
        objects is not transferred.

        @li If `*lhs.storage() == *rhs.storage()`,
        ownership of the underlying memory is swapped in
        constant time, with no possibility of exceptions.
        All iterators and references remain valid.

        @li If `*lhs.storage() != *rhs.storage()`,
        the contents are logically swapped by making a copy,
        which can throw. In this case all iterators and
        references are invalidated.

        @par Effects
        @code
        lhs.swap( rhs );
        @endcode

        @par Complexity
        Constant or linear in `lhs.size() + rhs.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param lhs The object to exchange.

        @param rhs The object to exchange.
        If `&lhs == &rhs`, this function call has no effect.

        @see @ref object::swap
    */
    friend
    void
    swap(object& lhs, object& rhs)
    {
        lhs.swap(rhs);
    }

    //------------------------------------------------------
    //
    // Lookup
    //
    //------------------------------------------------------

    /** Access the specified element, with bounds checking.

        Returns `boost::system::result` containing a reference to the
        mapped value of the element that matches `key`. Otherwise the result
        contains an `error_code`.

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.

        @par Complexity
        Constant on average, worst case linear in @ref size().
    */
    /** @{ */
    BOOST_JSON_DECL
    system::result<value&>
    try_at(string_view key) noexcept;

    BOOST_JSON_DECL
    system::result<value const&>
    try_at(string_view key) const noexcept;
    /** @} */

    /** Access the specified element, with bounds checking.

        Returns a reference to the mapped value of the element
        that matches `key`, otherwise throws.

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        Strong guarantee.

        @return A reference to the mapped value.

        @param key The key of the element to find.

        @param loc `source_location` to use in thrown exception; the source
            location of the call site by default.

        @throw `boost::system::system_error` if no such element exists.
    */
    /** @{ */
    inline
    value&
    at(
        string_view key,
        source_location const& loc = BOOST_CURRENT_LOCATION) &;

    inline
    value&&
    at(
        string_view key,
        source_location const& loc = BOOST_CURRENT_LOCATION) &&;

    BOOST_JSON_DECL
    value const&
    at(
        string_view key,
        source_location const& loc = BOOST_CURRENT_LOCATION) const&;
    /** @} */

    /** Access or insert the specified element

        Returns a reference to the value that is mapped
        to a key equivalent to key, performing an insertion
        of a null value if such key does not already exist.

        If an insertion occurs and results in a rehashing of the container, all
        iterators including any past-the-end iterators, and all references to
        the elements are invalidated. Otherwise, no iterators or references are
        invalidated.

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @return A reference to the mapped value.

        @param key The key of the element to find.
    */
    BOOST_JSON_DECL
    value&
    operator[](string_view key);

    /** Count the number of elements with a specific key

        This function returns the count of the number of
        elements match `key`. The only possible return values
        are 0 and 1.

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.
    */
    BOOST_JSON_DECL
    std::size_t
    count(string_view key) const noexcept;

    /** Find an element with a specific key

        This function returns an iterator to the element
        matching `key` if it exists, otherwise returns
        @ref end().

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.
    */
    BOOST_JSON_DECL
    iterator
    find(string_view key) noexcept;

    /** Find an element with a specific key

        This function returns a constant iterator to
        the element matching `key` if it exists,
        otherwise returns @ref end().

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.
    */
    BOOST_JSON_DECL
    const_iterator
    find(string_view key) const noexcept;

    /** Return `true` if the key is found

        This function returns `true` if a key with the
        specified string is found.

        @par Effects
        @code
        return this->find(key) != this->end();
        @endcode

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.

        @see @ref find
    */
    BOOST_JSON_DECL
    bool
    contains(string_view key) const noexcept;

    /** Return a pointer to the value if the key is found, or null

        This function searches for a value with the given
        key, and returns a pointer to it if found. Otherwise
        it returns null.

        @par Example
        @code
        if( auto p = obj.if_contains( "key" ) )
            std::cout << *p;
        @endcode

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.

        @see @ref find
    */
    BOOST_JSON_DECL
    value const*
    if_contains(string_view key) const noexcept;

    /** Return a pointer to the value if the key is found, or null

        This function searches for a value with the given
        key, and returns a pointer to it if found. Otherwise
        it returns null.

        @par Example
        @code
        if( auto p = obj.if_contains( "key" ) )
            std::cout << *p;
        @endcode

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.

        @see @ref find
    */
    BOOST_JSON_DECL
    value*
    if_contains(string_view key) noexcept;

    /** Return `true` if two objects are equal.

        Objects are equal when their sizes are the same,
        and when for each key in `lhs` there is a matching
        key in `rhs` with the same value.

        @par Complexity
        Constant, or linear (worst case quadratic) in `lhs.size()`.

        @par Exception Safety
        No-throw guarantee.
    */
    // inline friend speeds up overload resolution
    friend
    bool
    operator==(
        object const& lhs,
        object const& rhs) noexcept
    {
        return lhs.equal(rhs);
    }

    /** Return `true` if two objects are not equal.

        Objects are equal when their sizes are the same,
        and when for each key in `lhs` there is a matching
        key in `rhs` with the same value.

        @par Complexity
        Constant, or linear (worst case quadratic) in `lhs.size()`.

        @par Exception Safety
        No-throw guarantee.
    */
    // inline friend speeds up overload resolution
    friend
    bool
    operator!=(
        object const& lhs,
        object const& rhs) noexcept
    {
        return ! (lhs == rhs);
    }

    /** Serialize @ref object to an output stream.

        This function serializes an `object` as JSON into the output stream.

        @return Reference to `os`.

        @par Complexity
        Constant or linear in the size of `obj`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param os The output stream to serialize to.

        @param obj The value to serialize.
    */
    BOOST_JSON_DECL
    friend
    std::ostream&
    operator<<(
        std::ostream& os,
        object const& obj);
private:
#ifndef BOOST_JSON_DOCS
    // VFALCO friending a detail function makes it public
    template<class CharRange>
    friend
    std::pair<key_value_pair*, std::size_t>
    detail::find_in_object(
        object const& obj,
        CharRange key) noexcept;
#endif

    template<class InputIt>
    void
    construct(
        InputIt first,
        InputIt last,
        std::size_t min_capacity,
        std::input_iterator_tag);

    template<class InputIt>
    void
    construct(
        InputIt first,
        InputIt last,
        std::size_t min_capacity,
        std::forward_iterator_tag);

    template<class InputIt>
    void
    insert(
        InputIt first,
        InputIt last,
        std::input_iterator_tag);

    template<class InputIt>
    void
    insert(
        InputIt first,
        InputIt last,
        std::forward_iterator_tag);

    template< class... Args >
    std::pair<iterator, bool>
    emplace_impl(string_view key, Args&& ... args );

    BOOST_JSON_DECL
    key_value_pair*
    insert_impl(
        pilfered<key_value_pair> p,
        std::size_t hash);

    BOOST_JSON_DECL
    table*
    reserve_impl(std::size_t new_capacity);

    BOOST_JSON_DECL
    bool
    equal(object const& other) const noexcept;

    inline
    std::size_t
    growth(
        std::size_t new_size) const;

    inline
    void
    remove(
        index_t& head,
        key_value_pair& p) noexcept;

    inline
    void
    destroy() noexcept;

    inline
    void
    destroy(
        key_value_pair* first,
        key_value_pair* last) noexcept;

    template<class FS, class FB>
    auto
    do_erase(
        const_iterator pos,
        FS small_reloc,
        FB big_reloc) noexcept
        -> iterator;

    inline
    void
    reindex_relocate(
        key_value_pair* src,
        key_value_pair* dst) noexcept;
};

} // namespace json
} // namespace boost

#ifndef BOOST_JSON_DOCS
// boost::hash trait
namespace boost
{
namespace container_hash
{

template< class T > struct is_unordered_range;

template<>
struct is_unordered_range< json::object >
    : std::true_type
{};

} // namespace container_hash
} // namespace boost

// std::hash specialization
namespace std {
template <>
struct hash< ::boost::json::object > {
    BOOST_JSON_DECL
    std::size_t
    operator()(::boost::json::object const& jo) const noexcept;
};
} // std
#endif


// Must be included here for this file to stand alone
#include <boost/json/value.hpp>

// includes are at the bottom of <boost/json/value.hpp>

#endif
