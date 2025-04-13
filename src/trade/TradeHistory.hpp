#pragma once
#ifndef INCLUDE_TRADE_TRADEHISTORY_HPP
#define INCLUDE_TRADE_TRADEHISTORY_HPP

#include <Stock.hpp>
#include <deque>
#include <limits>

struct TradeHistory {
    StockCount amount;
    StockPrice price;
};

struct TickHistory {
    StockPrice start = {std::numeric_limits<Money_data_t>::min()};
    StockPrice end;
    StockPrice high = {std::numeric_limits<Money_data_t>::min()};
    StockPrice low = {std::numeric_limits<Money_data_t>::max()};
};
struct TradeBoard;

/**
 * 過去の取引を記録するclass
 *
 */
class TradeBoardHistory {
    struct {
        std::optional<StockPrice> higer;
        std::optional<StockPrice> lower;
        StockPrice latest;
    } CurrentStockPrice;

    std::deque<TradeHistory> history{{{300}, {100}}};
    std::deque<TickHistory> tick_history{};

    void update_current_high();
    void update_current_low();

    void update_current_high(StockPrice price);
    void update_current_low(StockPrice price);

    void update_current_high_add(StockPrice price);
    void update_current_low_add(StockPrice price);

    void update_current_latest(StockPrice price);
    void update_history(StockPrice price, StockCount count);
};

#endif  // INCLUDE_TRADE_TRADEHISTORY_HPP1
