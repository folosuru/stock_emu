#include "./Trader.hpp"

#include <cstdio>
#include <random>

#include "Stock.hpp"
#include "Trade.hpp"

namespace bot::trader {

namespace {
std::random_device seed_gen;
std::default_random_engine engine(seed_gen());
std::uniform_int_distribution<> action_dist(0, 2);
std::uniform_real_distribution<> amount_dist(0.0, 1.0);

constexpr inline StockPrice getMarketPriceHigher(const std::shared_ptr<TradeBoard> &board, StockPrice default_price,
                                                 int move_value) {
    const auto current = board->getCurrentPrice();
    if (current.higer) {
        return board->to_withen_PriceLimit(*current.higer);
    }

    if (current.lower) {
        return board->to_withen_PriceLimit(StockPrice{current.lower->getValue() + move_value});
    }
    return board->to_withen_PriceLimit(StockPrice(default_price.getValue()));
}

constexpr inline StockPrice getMarketPriceLower(const std::shared_ptr<TradeBoard> &board, StockPrice default_price,
                                                int move_value) {
    const auto current = board->getCurrentPrice();
    if (current.lower) {
        return board->to_withen_PriceLimit(*current.lower);
    }

    if (current.higer) {
        return board->to_withen_PriceLimit(StockPrice{current.higer->getValue() - move_value});
    }

    return board->to_withen_PriceLimit(StockPrice(default_price.getValue()));
}

}  // namespace

void Random::Trade(Trader &trader, StockMarketRef market) {
    std::uniform_int_distribution<> stock_select(0, market->count() - 1);
    StockId stock = stock_select(engine);
    auto action = action_dist(engine);
    // auto amount = 0.05 / (amount_dist(engine) + 0.2);

    auto board = (*market)[stock];

    switch (action) {
        case 0: {  // buy
            StockPrice price = getMarketPriceHigher(board, {0}, 1);
            auto count = StockCount{50};  // (trader.money / price) * amount;
            trader.buy(price, count, stock, market);
            break;
        }
        case 1: {  // sell
            StockPrice price = getMarketPriceLower(board, {0}, 1);
            auto count = StockCount{trader.stock[stock] >= 50 ? 50 : trader.stock[stock].to_StockCount()};
            // trader.stock[stock] * 0.05;  // amount;

            trader.sell(price, count, stock, market);
            break;
        }
        case 2: {  // ignore
                   // std::cout << "ignore \n" << std::endl;
            break;
        }
    }
}

void Fundamental::Trade(Trader &trader, StockMarketRef market) {
    std::discrete_distribution stock_select(market->get_value_dondake_hanareteru().begin(),
                                            market->get_value_dondake_hanareteru().end());
    StockId stock = stock_select(engine);
    auto board = market->get(stock);

    // std::cout << "value:" << board->StockValue().getValue() << ", price:" <<
    // board->getCurrentPrice().latest.getValue()
    //          << "\n";
    if (board->getCurrentPrice().higer && (board->getCurrentPrice().higer.value() < board->StockValue())) {
        //  std::cout << "low\n";
        StockPrice price = getMarketPriceHigher(board, board->StockValue(), 1);
        trader.buy(price, {50}, board->id, market);
    } else if (board->getCurrentPrice().lower && (board->getCurrentPrice().lower.value() > board->StockValue())) {
        //  std::cout << "high\n";
        StockPrice price = getMarketPriceLower(board, board->StockValue(), 1);
        auto count = StockCount{trader.stock[stock] >= 50 ? 50 : trader.stock[stock].to_StockCount()};
        trader.sell(price, count, board->id, market);
    }
}

}  // namespace bot::trader
