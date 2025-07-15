#pragma once
#ifndef INCLUDE_UTIL_FUZZY_FUZZY_HPP
#define INCLUDE_UTIL_FUZZY_FUZZY_HPP
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <ranges>
#include <vector>

template<class T, size_t Discrete_count_ = 8>
class Fuzzy {
public:
    constexpr static auto Discrete_count = Discrete_count_;
    using DiscreteArr_t = std::array<T, Discrete_count>;

    class FuzzySet {
    public:
        /*  hight |
         *        |       ^
         *        |      / \
         *        |     /   \
         *       0|____/_____\___
         *        |   ^   ^  ^
         *        start  top end
         */

        T start, top, end;
        T left_slope_rate, right_slope_rate;

    public:
        T getDiscreteValue(T hight, T value) const noexcept {
            if (value < start || end < value) {
                return 0;
            } else if (start <= value && value <= top) {
                return std::clamp((value - start) * left_slope_rate, 0.0, hight);
            } else {  // top < value  && value < right
                return std::clamp((value - end) * right_slope_rate, 0.0, hight);
            }
        }

        FuzzySet(T start_, T top_, T end_, T hight_) : start(start_), top(top_), end(end_) {
            constexpr T e = std::numeric_limits<T>::epsilon();
            if ((top - start) < e) {
                left_slope_rate = 0;
            } else {
                left_slope_rate = hight_ / (top - start);
            }

            if ((end - top) < e) {
                right_slope_rate = 0;
            } else {
                right_slope_rate = -hight_ / (end - top);
            }
        }
    };

    using FuzzySetVector_t = std::vector<FuzzySet>;
    using HightVector_t = std::vector<T>;

    static void BindAndDiscretization(const FuzzySetVector_t& FuzzySetSet, const HightVector_t& hightVec,
                                      const DiscreteArr_t& discretization_rate, DiscreteArr_t& result_out) {
        for (auto& i : result_out) {
            i = 0;
        }

        for (const auto& i : std::views::iota((size_t)0, FuzzySetSet.size())) {
            const auto& fuzzy_set = FuzzySetSet[i];
            const auto& hight = hightVec[i];

            for (auto index : std::views::iota((size_t)0, Discrete_count)) {
                result_out[index] =
                    std::max(fuzzy_set.getDiscreteValue(hight, discretization_rate[index]), result_out[index]);
            }
        }
    }

    static T getCenter(const DiscreteArr_t& x, const DiscreteArr_t& y) {
        T weight_sum = 0;
        for (const auto& i : y) {
            weight_sum += i;
        }

        T weighted_sum = 0;
        for (auto index : std::views::iota((size_t)0, Discrete_count)) {
            weighted_sum += x[index] * y[index];
        }
        return weighted_sum / weight_sum;
    }

    Fuzzy() = delete;
};

#endif  // INCLUDE_UTIL_FUZZY_FUZZY_HPP
