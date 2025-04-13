#pragma once
#ifndef INCLUDE_TRADE_PRICELIMIT_HPP
#define INCLUDE_TRADE_PRICELIMIT_HPP

#include <Stock.hpp>

class PriceLimit {
    struct PriceLimit_data {
        StockPrice PriceLimit_high;
        StockPrice PriceLimit_low;
    };

    std::optional<PriceLimit_data> PriceLimit = std::nullopt;

    StockPrice to_withen_PriceLimit(StockPrice pric) const noexcept;
    bool is_withen_PriceLimit(StockPrice price) const noexcept;
};

#endif  // INCLUDE_TRADE_PRICELIMIT_HPP1
