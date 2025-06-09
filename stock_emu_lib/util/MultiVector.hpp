#pragma once

#ifndef INCLUDE_UTIL_MULTIVECTOR_HPP_
#define INCLUDE_UTIL_MULTIVECTOR_HPP_
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace util {

template<class... T>
class MlutiVector {
    std::tuple<std::vector<T>...> data;

    constexpr static size_t Data_Colum_count = std::tuple_size<decltype(data)>::value;

public:
    void push(const T&... args) {
        push_helper<0>(args...);
    }

    template<size_t Index>
    const auto& getColumn() {
        return std::get<Index>(data);
    }

    void erase(size_t index) {
        erase_helper(index, std::make_index_sequence<Data_Colum_count>());
    }

    template<class Out_t>
    Out_t get_As(size_t index) {
        static_assert(std::is_aggregate_v<Out_t>, "get_As need construct by {T...}");
        return get_As_helper<Out_t>(index, std::make_index_sequence<Data_Colum_count>());
    }

    template<class Out_t>
    const Out_t get_As(size_t index) const {
        static_assert(std::is_aggregate_v<Out_t>, "get_As need construct by {T...}");
        return get_As_helper<Out_t>(index, std::make_index_sequence<Data_Colum_count>());
    }

private:
    template<class Out_t, class IndexT, IndexT... Seq>
    Out_t get_As_helper(size_t index, std::integer_sequence<IndexT, Seq...>) {
        return {std::get<Seq>(data)[index]...};
    }

    template<class VectorT>
    static void eraseVector(std::vector<VectorT>& vec, size_t index) {
        vec.erase(vec.begin() + index);
    }

    template<class IndexT, IndexT... Seq>
    void erase_helper(size_t index, std::integer_sequence<IndexT, Seq...>) {
        (..., eraseVector(std::get<Seq>(data), index));
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
};
}

#endif  // INCLUDE_UTIL_MULTIVECTOR_HPP_
