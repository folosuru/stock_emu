#pragma once
#ifndef BOT_TRADER_TRADER_HPP_
#define BOT_TRADER_TRADER_HPP_
#include <stock_emu_lib/TradeBot.hpp>
#include <stock_emu_lib/trade/Trade.hpp>

namespace bot::trader {

class Random {
public:
    void Trade(Trader&, StockMarketRef);
};

class Fundamental {
public:
    void Trade(Trader&, StockMarketRef);
};

class DayTrade {
public:
    void Trade(Trader&, StockMarketRef);
};

static_assert(TradeBot<Random>, "is not trader");
static_assert(TradeBot<Fundamental>, "is not trader");
}
#endif  // BOT_TRADER_TRADER_HPP_
