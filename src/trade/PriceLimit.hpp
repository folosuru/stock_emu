#pragma once
#ifndef INCLUDE_TRADE_PRICELIMIT_HPP
#define INCLUDE_TRADE_PRICELIMIT_HPP

#include <Stock.hpp>

class PriceLimit {
public:
    struct PriceLimit_data {
        StockPrice PriceLimit_high;
        StockPrice PriceLimit_low;
    };

    StockPrice to_withen_PriceLimit(StockPrice pric) const noexcept;
    bool is_withen_PriceLimit(StockPrice price) const noexcept;

    void update_PriceLimit(StockPrice price) noexcept;

    const auto& get() const noexcept {
        return PriceLimit;
    }

private:
    std::optional<PriceLimit_data> PriceLimit = std::nullopt;

    // high_limit = current + current * width_rate;
    // low_limit  = current - current * width_rate;
    constexpr static double width_rate = 0.3;

    // 価格が1とかになると、小数点以下切り捨ての結果、範囲が1~1になる
    // それを回避するために最低限の変化幅を用意する
    constexpr static int lower_limit_width = 30;
};

#endif  // INCLUDE_TRADE_PRICELIMIT_HPP1
