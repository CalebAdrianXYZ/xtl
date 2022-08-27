#if !defined(ADRIAN_XTL_OPTIONAL_HPP)
#define ADRIAN_XTL_OPTIONAL_HPP

// TODO: refactor constraints used in optional to use subsumable-concepts.
// TODO: further document STL dependencies.

#if defined(_MSC_VER)
#   pragma warning(push, 0)
#endif

// three_way_comparable_with
// equality_comparable_with
// strong_ordering
// common_comparison_category_t
#include <compare>
// *various*
#include <concepts>
// size_t
#include <cstddef>
// exception
#include <exception>
// initializer_list
#include <initializer_list>
// hash
// invocable
// invoke
// invoke_result_t
#include <functional>
// addressof
// construct_at
// destruct_at
#include <memory>
// *various*
#include <type_traits>
// exchange
#include <utility>

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4514) // unreferenced inline function removed
#   pragma warning(disable : 4820) // padding bytes added
#   pragma warning(disable : 4625) // copy constructor implicitly deleted
#   pragma warning(disable : 4626) // assignment operator implicitly deleted
#   pragma warning(disable : 5026) // move constructor implicitly deleted
#   pragma warning(disable : 5027) // move assignment operator implicitly deleted
#   pragma warning(disable : 4582) // constructor not implicitly called
#   pragma warning(disable : 4583) // destructor not implicitly called
#elif defined(__GNUC__)
#   pragma GCC diagnostic push
#   if defined(__clang__)
#       pragma GCC diagnostic ignored "-Wc++98-compat"
#       pragma GCC diagnostic ignored "-Wc++98-compat-pedantic"
#       pragma GCC diagnostic ignored "-Wweak-vtables"
#   endif
#   pragma GCC diagnostic ignored "-Wpadded"
#endif

#if defined(_MSC_VER)
#define ADRIAN_XTL_OPTIONAL_ASSUME(...) __assume(__VA_ARGS__)
#elif defined(__GNUC__) or defined(__clang__)
#define ADRIAN_XTL_OPTIONAL_ASSUME(...) \
    do { if (not (__VA_ARGS__)) { __builtin_unreachable(); } } while(0)
#else
#define ADRIAN_XTL_OPTIONAL_ASSUME(...) (void)(__VA_ARGS__);
#endif

#define ADRIAN_XTL_OPTIONAL_MOVE(...) \
    static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)

#define ADRIAN_XTL_OPTIONAL_FORWARD(...) \
    static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace adrian::xtl
{
    struct bad_optional_access : std::exception
    {
        virtual auto what() const noexcept -> char const* override
        {
            return "bad optional access";
        }
    };

    struct nullopt_t final
    {
        explicit constexpr nullopt_t(int) {}
    };
    inline constexpr auto nullopt = nullopt_t{0};

    struct in_place_t final
    {
        explicit in_place_t() = default;
    } inline constexpr in_place{};

    template<typename T>
    struct in_place_type_t final
    {
        explicit in_place_type_t() = default;
    };
    template<typename T>
    inline constexpr auto in_place_type = in_place_type_t<T>{};

    template<std::size_t I>
    struct in_place_index_t final
    {
        explicit in_place_index_t() = default;
    };
    template<std::size_t I>
    inline constexpr auto in_place_index = in_place_index_t<I>{};

    namespace _optional
    {
        template<typename T>
        concept cpp17_destructible
            = std::is_nothrow_destructible_v<T>
            and (not std::is_reference_v<T>)
            and (not std::is_array_v<T>);
    }

    template<_optional::cpp17_destructible T>
    struct optional;
}

namespace adrian::xtl::_optional
{
    template<typename T>
    concept copy_constructible
        = std::is_copy_constructible_v<T>;

    template<typename T>
    concept trivially_copy_constructible
        = copy_constructible<T>
        and std::is_trivially_copy_constructible_v<T>;

    template<typename T>
    concept move_constructible
        = std::is_move_constructible_v<T>;

    template<typename T>
    concept trivially_move_constructible
        = move_constructible<T>
        and std::is_trivially_move_constructible_v<T>;

    template<typename T>
    concept copy_assignable
        = copy_constructible<T>
        and std::is_copy_assignable_v<T>;

    template<typename T>
    concept trivially_copy_assignable
        = copy_assignable<T>
        and trivially_copy_constructible<T>
        and std::is_trivially_copy_assignable_v<T>;

    template<typename T>
    concept move_assignable
        = move_constructible<T>
        and std::is_move_assignable_v<T>;

    template<typename T>
    concept trivially_move_assignable
        = move_assignable<T>
        and trivially_move_constructible<T>
        and std::is_trivially_move_assignable_v<T>;

    template<typename O, typename T, typename Opt>
    concept _conversion_constructible
        = (std::same_as<Opt const&, O> or std::same_as<Opt&&, O>)
        and requires { typename Opt::value_type; }
        and std::same_as<optional<typename Opt::value_type>, Opt>
        and (not std::same_as<typename Opt::value_type, T>)
        and (std::is_constructible_v<T, decltype(*std::declval<O>())>)
        and (not std::is_constructible_v<T, Opt&>)
        and (not std::is_constructible_v<T, Opt const&>)
        and (not std::is_constructible_v<T, Opt&&>)
        and (not std::is_constructible_v<T, Opt const&&>)
        and (not std::is_convertible_v<Opt&, T>)
        and (not std::is_convertible_v<Opt const&, T>)
        and (not std::is_convertible_v<Opt&&, T>)
        and (not std::is_convertible_v<Opt const&&, T>);

    template<typename O, typename T>
    concept conversion_constructible
        = _conversion_constructible<
            O,
            T,
            std::remove_const_t<std::remove_reference_t<O>>>;

    template<typename O, typename T, typename Opt>
    concept _conversion_assignable
        = _conversion_constructible<O, T, Opt>
        and (std::is_assignable_v<T&, decltype(*std::declval<O>())>)
        and (not std::is_assignable_v<T&, Opt&>)
        and (not std::is_assignable_v<T&, Opt const&>)
        and (not std::is_assignable_v<T&, Opt&&>)
        and (not std::is_assignable_v<T&, Opt const&&>);

    template<typename O, typename T>
    concept conversion_assignable
        = _conversion_assignable<
            O,
            T,
            std::remove_const_t<std::remove_reference_t<O>>>;

#if defined(_MSC_VER)
    [[noreturn]] __declspec(noinline)
#else
    [[gnu::cold, gnu::noinline, noreturn]]
#endif
    inline auto throw_bad_optional() -> void
    {
        throw bad_optional_access();
    }

    struct and_then_func final
    {
        template<
            typename QO,
            typename F,
            typename O = std::remove_const_t<std::remove_reference_t<QO>>,
            typename T = typename O::value_type,
            typename QT = decltype(*std::declval<QO>()),
            typename R = std::invoke_result_t<F, QT>,
            typename U = typename R::value_type>
        requires
            std::same_as<O, optional<T>> and std::same_as<R, optional<U>>
        constexpr auto operator()(QO&& o, F&& f) const -> R
        {
            if (o) {
                return std::invoke(
                    ADRIAN_XTL_OPTIONAL_FORWARD(f),
                    *ADRIAN_XTL_OPTIONAL_FORWARD(o));
            }
            else {
                return R();
            }
        }
    } inline constexpr and_then{};

    struct transform_func final
    {
        template<
            typename QO,
            typename F,
            typename O = std::remove_const_t<std::remove_reference_t<QO>>,
            typename T = typename O::value_type,
            typename QT = decltype(*std::declval<QO>()),
            typename QU = std::invoke_result_t<F, QT>,
            typename U = std::remove_const_t<std::remove_reference_t<QU>>,
            typename R = optional<U>>
        requires
            std::same_as<O, optional<T>>
            and (not std::same_as<U, in_place_t>)
            and (not std::same_as<U, nullopt_t>)
            and (not std::is_array_v<U>)
            and std::constructible_from<U, QU>
        constexpr auto operator()(QO&& o, F&& f) const -> R
        {
            if (o) {
                return R(
                    in_place,
                    std::invoke(
                        ADRIAN_XTL_OPTIONAL_FORWARD(f),
                        *ADRIAN_XTL_OPTIONAL_FORWARD(o)));
            }
            else {
                return R();
            }
        }
    } inline constexpr transform{};

    struct or_else_func final
    {
        template<
            typename QO,
            typename F,
            typename O = std::remove_const_t<std::remove_reference_t<QO>>,
            typename T = typename O::value_type,
            typename QT = decltype(*std::declval<QO>()),
            typename QR = std::invoke_result_t<F>,
            typename R = std::remove_const_t<std::remove_reference_t<QR>>>
        requires
            std::same_as<O, optional<T>> and std::same_as<R, optional<T>>
            and std::constructible_from<T, QT>
        constexpr auto operator()(QO&& o, F&& f) const -> optional<T>
        {
            if (o) {
                return *ADRIAN_XTL_OPTIONAL_FORWARD(o);
            }
            else {
                return std::invoke(ADRIAN_XTL_OPTIONAL_FORWARD(f));
            }
        }
    } inline constexpr or_else{};
}

namespace adrian::xtl
{
    template<_optional::cpp17_destructible T>
    struct optional
    {
    private: // private member types
        union storage_type
        {
            char empty{};
            T value;

            constexpr storage_type() noexcept {}

            ~storage_type()
                requires std::is_trivially_destructible_v<T>
                = default;

            ~storage_type()
                requires (not std::is_trivially_destructible_v<T>)
            {
            }
        };

    private: // private data
        bool m_exists{};
        storage_type m_storage{};

    public: // member types
        using value_type = T;

    public: // constructors

        // 1

        optional() = default;

        constexpr optional(nullopt_t) noexcept {}

        // 2

        optional(optional const& other)
            = delete;

        optional(optional const& other)
            requires _optional::trivially_copy_constructible<T>
            = default;

        constexpr optional(optional const& other)
            requires _optional::copy_constructible<T>
        {
            if (other) {
                emplace(*other);
            }
        }

        // 3

        optional(optional&& other)
            requires _optional::trivially_move_constructible<T>
            = default;

        constexpr optional(optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
            requires _optional::move_constructible<T>
        {
            if (other) {
                emplace(*ADRIAN_XTL_OPTIONAL_MOVE(other));
            }
        }

        // 4, 5

        template<typename O, typename Opt = std::remove_const_t<std::remove_reference_t<O>>>
        requires
            (std::same_as<Opt const&, O> or std::same_as<Opt&&, O>)
            and requires { typename Opt::value_type; }
            and std::same_as<optional<typename Opt::value_type>, Opt>
            and (not std::same_as<typename Opt::value_type, value_type>)
            and (std::is_constructible_v<T, decltype(*std::declval<O>())>)
            and (not std::is_constructible_v<T, Opt&>)
            and (not std::is_constructible_v<T, Opt const&>)
            and (not std::is_constructible_v<T, Opt&&>)
            and (not std::is_constructible_v<T, Opt const&&>)
            and (not std::is_convertible_v<Opt&, T>)
            and (not std::is_convertible_v<Opt const&, T>)
            and (not std::is_convertible_v<Opt&&, T>)
            and (not std::is_convertible_v<Opt const&&, T>)
        explicit(not std::is_convertible_v<typename Opt::value_type, T>)
        constexpr optional(O&& other)
        {
            if (other) {
                emplace(*ADRIAN_XTL_OPTIONAL_FORWARD(other));
            }
        }

        // 6

        template<typename... Args>
        requires std::constructible_from<T, Args...>
        explicit constexpr optional(in_place_t, Args&&... args)
        {
            emplace(ADRIAN_XTL_OPTIONAL_FORWARD(args)...);
        }

        // 7

        template<typename U, typename... Args>
        requires std::constructible_from<T, std::initializer_list<U>&, Args...>
        explicit constexpr optional(in_place_t,
            std::initializer_list<U> ilist, Args&&... args)
        {
            emplace(ilist, ADRIAN_XTL_OPTIONAL_FORWARD(args)...);
        }

        // 8

        template<typename U>
        requires std::constructible_from<T, U>
            and (not std::same_as<std::remove_cvref_t<U>, in_place_t>)
            and (not std::same_as<std::remove_cvref_t<U>, optional>)
        explicit(not std::is_convertible_v<U, T>)
        constexpr optional(U&& value)
        {
            emplace(ADRIAN_XTL_OPTIONAL_FORWARD(value));
        }

    public: // destructors
        constexpr ~optional()
            requires (std::is_trivially_destructible_v<T>)
            = default;

        constexpr ~optional()
            requires (not std::is_trivially_destructible_v<T>)
        {
            reset();
        }

    public: // assignment operators

        // 1

        constexpr auto operator=(nullopt_t) noexcept -> optional&
        {
            reset();
            return *this;
        }

        // 2

        optional& operator=(optional const&)
            = delete;

        optional& operator=(optional const&)
            requires _optional::trivially_copy_assignable<T>
            = default;

        constexpr optional& operator=(optional const& other)
            requires _optional::copy_assignable<T>
        {
            if (m_exists) {
                if (other.m_exists) {
                    m_storage.value = *other;
                }
                else {
                    reset();
                }
            }
            else if (other.m_exists) {
                emplace(*other);
            }
            return *this;
        }

        // 3

        optional& operator=(optional&&)
            requires _optional::trivially_move_assignable<T>
            = default;

        constexpr optional& operator=(optional&& other)
            noexcept(std::is_nothrow_move_constructible_v<T>
                and std::is_nothrow_move_assignable_v<T>)
            requires _optional::move_assignable<T>
        {
            if (m_exists) {
                if (other.m_exists) {
                    m_storage.value = *ADRIAN_XTL_OPTIONAL_MOVE(other);
                }
                else {
                    reset();
                }
            }
            else if (other.m_exists) {
                emplace(*ADRIAN_XTL_OPTIONAL_MOVE(other));
            }
            return *this;
        }

        // 4

        template<typename U>
        requires
            std::is_constructible_v<T, U>
            and std::is_assignable_v<T&, U>
            and (not std::same_as<std::remove_cvref_t<U>, optional>)
            and (not std::same_as<std::decay_t<U>, T>
                or (not std::is_scalar_v<std::decay_t<T>>))
        constexpr auto operator=(U&& value) -> optional&
        {
            if (m_exists) {
                m_storage.value = value;
            }
            else {
                emplace(ADRIAN_XTL_OPTIONAL_FORWARD(value));
            }
            return *this;
        }

        // 5, 6

        template<
            typename O,
            typename Opt = std::remove_const_t<std::remove_reference_t<O>>>
        requires
            (std::same_as<Opt const&, O> or std::same_as<Opt&&, O>)
            and requires { typename Opt::value_type; }
            and std::same_as<optional<typename Opt::value_type>, Opt>
            and (not std::same_as<typename Opt::value_type, value_type>)
            and (std::is_constructible_v<T, decltype(*std::declval<O>())>)
            and (std::is_assignable_v<T&, decltype(*std::declval<O>())>)
            and (not std::is_constructible_v<T, Opt&>)
            and (not std::is_constructible_v<T, Opt const&>)
            and (not std::is_constructible_v<T, Opt&&>)
            and (not std::is_constructible_v<T, Opt const&&>)
            and (not std::is_convertible_v<Opt&, T>)
            and (not std::is_convertible_v<Opt const&, T>)
            and (not std::is_convertible_v<Opt&&, T>)
            and (not std::is_convertible_v<Opt const&&, T>)
            and (not std::is_assignable_v<T&, Opt&>)
            and (not std::is_assignable_v<T&, Opt const&>)
            and (not std::is_assignable_v<T&, Opt&&>)
            and (not std::is_assignable_v<T&, Opt const&&>)
        constexpr auto operator=(O&& other) -> optional&
        {
            if (m_exists) {
                if (other.m_exists) {
                    m_storage.value = *ADRIAN_XTL_OPTIONAL_FORWARD(other);
                }
                else {
                    reset();
                }
            }
            else if (other.m_exists) {
                emplace(*ADRIAN_XTL_OPTIONAL_FORWARD(other));
            }
            return *this;
        }

    public: // observers
        constexpr auto operator->() const -> T const*
        {
            ADRIAN_XTL_OPTIONAL_ASSUME(m_exists);
            return std::addressof(m_storage.value);
        }

        constexpr auto operator->() -> T*
        {
            ADRIAN_XTL_OPTIONAL_ASSUME(m_exists);
            return std::addressof(m_storage.value);
        }

        constexpr auto operator*() const& -> T const&
        {
            ADRIAN_XTL_OPTIONAL_ASSUME(m_exists);
            return m_storage.value;
        }

        constexpr auto operator*() & -> T&
        {
            ADRIAN_XTL_OPTIONAL_ASSUME(m_exists);
            return m_storage.value;
        }

        constexpr auto operator*() const&& -> T const&&
        {
            ADRIAN_XTL_OPTIONAL_ASSUME(m_exists);
            return static_cast<T const&&>(m_storage.value);
        }

        constexpr auto operator*() && -> T&&
        {
            ADRIAN_XTL_OPTIONAL_ASSUME(m_exists);
            return static_cast<T&&>(m_storage.value);
        }

        explicit constexpr operator bool() const noexcept { return m_exists; }
        constexpr auto has_value() const noexcept -> bool { return m_exists; }

        // value

        constexpr auto value() const& -> T const&
        {
            if (m_exists) {
                return m_storage.value;
            }
            else {
                _optional::throw_bad_optional();
            }
        }

        constexpr auto value() & -> T&
        {
            if (m_exists) {
                return m_storage.value;
            }
            else {
                _optional::throw_bad_optional();
            }
        }

        constexpr auto value() const&& -> T const&&
        {
            if (m_exists) {
                return static_cast<T const&&>(m_storage.value);
            }
            else {
                _optional::throw_bad_optional();
            }
        }

        constexpr auto value() && -> T&&
        {
            if (m_exists) {
                return static_cast<T&&>(m_storage.value);
            }
            else {
                _optional::throw_bad_optional();
            }
        }

        // value or

        template<typename U>
        constexpr auto value_or(U&& value) const& -> T
        {
            return *this
                ? **this
                : static_cast<T>(static_cast<U&&>(value));
        }

        template<typename U>
        constexpr auto value_or(U&& value) && -> T
        {
            return *this
                ? *ADRIAN_XTL_OPTIONAL_MOVE(*this)
                : static_cast<T>(static_cast<U&&>(value));
        }

    public: // monadic operations

        // and_then

        template<typename F>
        requires std::invocable<_optional::and_then_func, optional const&, F>
        constexpr auto and_then(F&& f) const&
        {
            return _optional::and_then(*this, ADRIAN_XTL_OPTIONAL_FORWARD(f));
        }

        template<typename F>
        requires std::invocable<_optional::and_then_func, optional&, F>
        constexpr auto and_then(F&& f) &
        {
            return _optional::and_then(*this, ADRIAN_XTL_OPTIONAL_FORWARD(f));
        }

        template<typename F>
        requires std::invocable<_optional::and_then_func, optional const&&, F>
        constexpr auto and_then(F&& f) const&&
        {
            return _optional::and_then(
                static_cast<optional const&&>(*this),
                ADRIAN_XTL_OPTIONAL_FORWARD(f));
        }

        template<typename F>
        requires std::invocable<_optional::and_then_func, optional&&, F>
        constexpr auto and_then(F&& f) &&
        {
            return _optional::and_then(
                static_cast<optional&&>(*this),
                ADRIAN_XTL_OPTIONAL_FORWARD(f));
        }

        // transform

        template<typename F>
        requires std::invocable<_optional::transform_func, optional const&, F>
        constexpr auto transform(F&& f) const&
        {
            return _optional::transform(*this, ADRIAN_XTL_OPTIONAL_FORWARD(f));
        }

        template<typename F>
        requires std::invocable<_optional::transform_func, optional&, F>
        constexpr auto transform(F&& f) &
        {
            return _optional::transform(*this, ADRIAN_XTL_OPTIONAL_FORWARD(f));
        }

        template<typename F>
        requires std::invocable<_optional::transform_func, optional const&&, F>
        constexpr auto transform(F&& f) const&&
        {
            return _optional::transform(
                static_cast<optional const&&>(*this),
                ADRIAN_XTL_OPTIONAL_FORWARD(f));
        }

        template<typename F>
        requires std::invocable<_optional::transform_func, optional&&, F>
        constexpr auto transform(F&& f) &&
        {
            return _optional::transform(
                static_cast<optional&&>(*this),
                ADRIAN_XTL_OPTIONAL_FORWARD(f));
        }

        // or_else

        template<typename F>
        requires std::invocable<_optional::or_else_func, optional const&, F>
        constexpr auto or_else(F&& f) const& -> optional
        {
            return _optional::or_else(*this, ADRIAN_XTL_OPTIONAL_FORWARD(f));
        }

        template<typename F>
        requires std::invocable<_optional::or_else_func, optional&&, F>
        constexpr auto or_else(F&& f) && -> optional
        {
            return _optional::or_else(
                static_cast<optional&&>(*this),
                ADRIAN_XTL_OPTIONAL_FORWARD(f));
        }

    private: // private modifiers
        template<typename... Args>
        requires std::constructible_from<T, Args...>
        constexpr auto _emplace(Args&&... args)
            noexcept(std::is_nothrow_constructible_v<T, Args...>)
            -> T&
        {
            reset();
            auto ptr = std::construct_at(
                std::addressof(m_storage.value),
                ADRIAN_XTL_OPTIONAL_FORWARD(args)...);
            m_exists = true;
            return *ptr;
        }

    public: // modifiers
        constexpr auto swap(optional& other)
            // TODO: not noexcept correct
            noexcept(std::is_nothrow_swappable_v<T>)
            -> void
            requires (std::is_swappable_v<T>)
        {
            if (this == &other) { return; }

            using std::swap;

            if (m_exists) {
                if (other.m_exists) {
                    swap(m_storage.value, other.m_storage.value);
                }
                else {
                    other.emplace(ADRIAN_XTL_OPTIONAL_MOVE(m_storage.value));
                    reset();
                }
            } else if (other.m_exists) {
                emplace(ADRIAN_XTL_OPTIONAL_MOVE(other.m_storage.value));
                other.reset();
            }
        }

        constexpr auto reset() noexcept -> void
        {
            if (std::exchange(m_exists, false)) {
                std::destroy_at(std::addressof(m_storage.value));
            }
        }

        template<typename... Args>
        requires std::constructible_from<T, Args...>
        constexpr auto emplace(Args&&... args)
            noexcept(std::is_nothrow_constructible_v<T, Args...>)
            -> T&
        {
            return _emplace(ADRIAN_XTL_OPTIONAL_FORWARD(args)...);
        }

        template<typename U, typename... Args>
        requires std::constructible_from<T, std::initializer_list<U>&, Args...>
        constexpr auto emplace(std::initializer_list<U> ilist, Args&&... args)
            noexcept(std::is_nothrow_constructible_v<T,
                std::initializer_list<U>&, Args...>)
            -> T&
        {
            return _emplace(ilist, ADRIAN_XTL_OPTIONAL_FORWARD(args)...);
        }

    public: // hidden friends
        friend constexpr auto swap(optional& x, optional& y) -> void
        {
            x.swap(y);
        }
    };

    template<typename T> optional(T) -> optional<T>;

    template<typename T, typename U>
    requires std::equality_comparable_with<T const&, U const&>
    constexpr auto operator==(optional<T> const& x, optional<U> const& y)
        -> bool
    {
        if (x and y) {
            return *x == *y;
        }
        return !x and !y;
    }

    template<typename T, typename U>
    requires std::three_way_comparable_with<T const&, U const&>
    constexpr auto operator<=>(optional<T> const& x, optional<U> const& y)
        -> std::common_comparison_category_t<T, U>
    {
        if (!x) {
            if (y) {
                return std::strong_ordering::less;
            }
            else {
                return std::strong_ordering::equal;
            }
        }
        else if (!y) {
            return std::strong_ordering::greater;
        }
        else {
            return *x <=> *y;
        }
    }

    template<typename T, typename..., typename DT = std::decay_t<T>>
    constexpr auto make_optional(T&& value)
        -> optional<DT>
    {
        return optional<DT>(ADRIAN_XTL_OPTIONAL_FORWARD(value));
    }

    template<typename T, typename... Args, typename DT = std::decay_t<T>>
    constexpr auto make_optional(Args&&... args)
        -> optional<DT>
    {
        return optional<DT>(ADRIAN_XTL_OPTIONAL_FORWARD(args)...);
    }

    template<
        typename T, typename U, typename... Args,
        typename DT = std::decay_t<T>>
    constexpr auto make_optional(std::initializer_list<U> ilist, Args&&... args)
        -> optional<DT>
    {
        return optional<DT>(ilist, ADRIAN_XTL_OPTIONAL_FORWARD(args)...);
    }
}

template<typename T>
requires std::default_initializable<std::hash<T>>
struct std::hash<adrian::xtl::optional<T>>
{
    constexpr auto operator()(adrian::xtl::optional<T> const& o) const -> std::size_t
    {
        return o ? hash<T>()(*o) : size_t(-1);
    }
};

#undef ADRIAN_XTL_OPTIONAL_ASSUME
#undef ADRIAN_XTL_OPTIONAL_MOVE
#undef ADRIAN_XTL_OPTIONAL_FORWARD

#if defined(_MSC_VER)
#   pragma warning(pop)
#elif defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif

#endif
