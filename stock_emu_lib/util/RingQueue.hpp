#pragma once

#ifndef INCLUDE_UTIL_RINGQUEUE_HPP_
#define INCLUDE_UTIL_RINGQUEUE_HPP_

#include <array>
#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace util {

template<class... p>
struct parameters {};

template<class>
struct func_to_arg;

template<class t, class... args>
t get_front();

template<class R, class... Args>
struct func_to_arg<R (*)(Args...)> {
    using arg = std::tuple<Args...>;
    using front = decltype(get_front<Args...>());
    using return_t = R;
};

template<class T, class U>
concept destructor = requires {
    &T::run;

    // 関数runの戻り値はvoid
    { std::declval<typename func_to_arg<decltype(&T::run)>::return_t>() } -> std::same_as<void>;

    // 第一引数はU&
    { std::declval<std::add_lvalue_reference_t<typename func_to_arg<decltype(&T::run)>::front>>() } -> std::same_as<U>;
};

template<class T, size_t N, destructor<T&> destructor_t>
class RingQueue {
public:
    constexpr T& get(size_t index) {
        if (index >= N) {
            // exception!
        }
        return data[getIndex(index)];
    }

    template<class... Args>
    constexpr void destruct(Args&&... args) {
        destructor_t::run(data[current], args...);
        if (current + 1 == N) {
            current = 0;
        } else {
            current++;
        }
    }

private:
    constexpr size_t getIndex(size_t index) {
        if (current + index >= N) {
            return current + index - N;
        } else {
            return current + index;
        }
    }

    std::array<T, N> data;
    size_t current = 0;
    /*
        static_assert([]() {
            struct obj {
                constexpr static void run(int&) {
                    return;
                }
            };
            RingQueue<int, 5, obj> q;
            if (q.getIndex(3) != 3) return false;
            q.destruct();
            if (q.getIndex(4) != 0) return false;
            return true;
        }());
    */
};
}

#endif  // INCLUDE_UTIL_RINGQUEUE_HPP_
