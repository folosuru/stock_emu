#pragma once

#ifndef INCLUDE_UTIL_MULTIVECTOR_HPP_
#define INCLUDE_UTIL_MULTIVECTOR_HPP_
#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

namespace util {
#if __cplusplus >= 202002L
template<class OutT, class... DataT>
concept OutT_check = requires(DataT... args) { OutT{args...}; };
#else
template<class OutT, class... args>
class OutT_check_class {
    template<class OutT_, class... args_, typename = decltype(OutT_{std::declval<args_>()...})>
    static std::true_type chk();

    template<typename, typename...>
    static std::false_type chk();

public:
    static constexpr bool value = decltype(chk<OutT, args...>())::value;
};
template<class OutT, class... args>
constexpr bool OutT_check = OutT_check_class<OutT, args...>::value;

#endif
template<class... T>
class MultiVector {
    std::tuple<std::vector<T>...> data;
    size_t count = 0;

    constexpr static size_t Data_Colum_count = sizeof...(T);

public:
    size_t getRowCount() const noexcept {
        return count;
    }

    void push(const T&... args) {
        count++;
        push_helper<0>(args...);
    }

    void push(const std::tuple<T...>& arg) {
        count++;
        push_helper(arg, std::make_index_sequence<Data_Colum_count>());
    }

    template<size_t Index>
    auto& getColumn() {
        return std::get<Index>(data);
    }

    template<size_t Index>
    const auto& getColumn() const {
        return std::get<Index>(data);
    }

    template<size_t column>
    const auto& getItem(size_t row) const {
        return getColumn<column>()[row];
    }

    template<size_t column>
    auto& getItem(size_t row) {
        return getColumn<column>()[row];
    }

    std::tuple<T...> getRow(size_t index) const {
        return get_As_helper<std::tuple<T...>>(index, std::make_index_sequence<Data_Colum_count>());
    }

    template<class Out_t>
    Out_t get_As(size_t index) {
        constexpr auto is_valid = OutT_check<Out_t, T&...>;
        static_assert(is_valid, "get_As parameter type must need construct by {T&...}");
        if constexpr (is_valid) {
            return get_As_helper<Out_t>(index, std::make_index_sequence<Data_Colum_count>());
        }
    }

    template<class Out_t>
    const Out_t get_As(size_t index) const {
        constexpr auto is_valid = OutT_check<Out_t, const T&...>;
        static_assert(is_valid, "get_As parameter type must need construct by {const T&...}.");
        if constexpr (is_valid) {
            return get_As_helper<Out_t>(index, std::make_index_sequence<Data_Colum_count>());
        }
    }

private:
    template<class Out_t, class IndexT, IndexT... Seq>
    Out_t get_As_helper(size_t index, std::integer_sequence<IndexT, Seq...>) {
        return {std::get<Seq>(data)[index]...};
    }

    template<class Out_t, class IndexT, IndexT... Seq>
    Out_t get_As_helper(size_t index, std::integer_sequence<IndexT, Seq...>) const {
        return {std::get<Seq>(data)[index]...};
    }

    template<size_t index, class Front>
    void push_helper(const Front& front) {
        std::get<index>(data).push_back(front);
    }

    template<size_t index, class Front, class... Tail>
    void push_helper(const Front& front, const Tail&... tail) {
        std::get<index>(data).push_back(front);
        push_helper<index + 1>(tail...);
    }

    template<class IndexT, IndexT... Seq>
    void push_helper(std::tuple<T...> arg, std::integer_sequence<IndexT, Seq...>) {
        (..., std::get<Seq>(data).push_back(std::get<Seq>(arg)));
    }
};
}

#endif  // INCLUDE_UTIL_MULTIVECTOR_HPP_
