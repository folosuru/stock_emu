#pragma once
#include "stock_emu_lib/util/MultiVector.hpp"
#ifndef INCLUDE_TRADE_TRADEHISTORY_HPP
#define INCLUDE_TRADE_TRADEHISTORY_HPP

#include <deque>
#include <limits>
#include <stock_emu_lib/Stock.hpp>
#include <stock_emu_lib/money.hpp>

struct TradeHistory {
    StockCount amount;
    StockPrice price;
};

struct TickHistory {
    StockPrice start = {nothing};
    StockPrice end = {nothing};
    StockPrice high = {std::numeric_limits<Money_data_t>::min()};
    StockPrice low = {std::numeric_limits<Money_data_t>::max()};

    constexpr static Money_data_t nothing = std::numeric_limits<Money_data_t>::min();
};

struct TradeBoard;

/**
 * 過去の取引を記録するclass
 *
 */
class TradeBoardHistory {
public:
    constexpr const auto& getCurrentPrice() const noexcept {
        return CurrentStockPrice;
    }

    /**
     * 約定したときに呼ぶ
     */
    void update_current_high(StockPrice price, const TradeBoard& board);
    void update_current_low(StockPrice price, const TradeBoard& board);

    /**
     * 板に注文が追加された時に呼ぶ
     */
    void update_sell_board_add(StockPrice price);
    void update_buy_board_add(StockPrice price);

    void update_history(StockPrice price, StockCount count);

    const TickHistory tick();

private:
    struct {
        std::optional<StockPrice> higer;
        std::optional<StockPrice> lower;
        StockPrice latest = {0};
    } CurrentStockPrice;

    TickHistory currentTick;

    std::deque<TradeHistory> history{{{300}, {100}}};
    util::MultiVector<StockPrice, StockPrice, StockPrice, StockPrice> tick_history;
};
#endif  // INCLUDE_TRADE_TRADEHISTORY_HPP
