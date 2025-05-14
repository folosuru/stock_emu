#pragma once
#ifndef INCLUDE_TRADE_BOT_HPP
#define INCLUDE_TRADE_BOT_HPP
#include <concepts>
#include <stock_emu_lib/trade/Trade.hpp>

template<class T>
concept TradeBot = requires(T &x, Trader &treader, StockMarketRef &market) {
    { x.Trade(treader, market) } -> std::same_as<void>;
};

#endif  // INCLUDE_TRADE_BOT_HPP
