#include "./Trader.hpp"

#include <Stock.hpp>
#include <cstdio>
#include <random>
#include <trade/Trade.hpp>

#include "money.hpp"

namespace bot::trader {

namespace {
std::random_device seed_gen;
// std::default_random_engine engine(seed_gen());
std::default_random_engine engine(64);
std::uniform_int_distribution<> action_dist(0, 2);
std::uniform_real_distribution<> amount_dist(0.0, 1.0);

constexpr inline StockPrice getMarketPriceHigher(const std::shared_ptr<TradeBoard> &board, StockPrice default_price,
                                                 int move_value) {
    const auto current = board->getCurrentPrice();
    if (current.higer) {
        return board->getPriceLimit().to_withen_PriceLimit(*current.higer);
    }

    if (current.lower) {
        return board->getPriceLimit().to_withen_PriceLimit(StockPrice{current.lower->getValue() + move_value});
    }
    return board->getPriceLimit().to_withen_PriceLimit(StockPrice(default_price.getValue()));
}

constexpr inline StockPrice getMarketPriceLower(const std::shared_ptr<TradeBoard> &board, StockPrice default_price,
                                                int move_value) {
    const auto current = board->getCurrentPrice();
    if (current.lower) {
        return board->getPriceLimit().to_withen_PriceLimit(*current.lower);
    }

    if (current.higer) {
        return board->getPriceLimit().to_withen_PriceLimit(StockPrice{current.higer->getValue() - move_value});
    }

    return board->getPriceLimit().to_withen_PriceLimit(StockPrice(default_price.getValue()));
}

Money_data_t getTradeAmount() {
    return static_cast<Money_data_t>((10 * amount_dist(engine)) + 45);
}

constexpr Money_data_t default_price_value = 100000;

}  // namespace

void Random::Trade(Trader &trader, StockMarketRef market) {
    std::uniform_int_distribution<> stock_select(0, market->count() - 1);
    StockId stock = stock_select(engine);
    auto action = action_dist(engine);
    // auto amount = 0.05 / (amount_dist(engine) + 0.2);

    auto board = (*market)[stock];

    switch (action) {
        case 0: {  // buy
            StockPrice price = getMarketPriceHigher(board, {default_price_value}, 1);
            StockCount count = {getTradeAmount()};  // (trader.money / price) * amount;
            trader.buy(price, count, stock, market);
            break;
        }
        case 1: {  // sell
            StockPrice price = getMarketPriceLower(board, {default_price_value}, 1);
            auto count_tmp = getTradeAmount();
            auto count = StockCount{trader.stock[stock] >= count_tmp ? count_tmp : trader.stock[stock].to_StockCount()};
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
    //

    if (board->getCurrentPrice().higer) {
        // StockValue よりも安く買える
        if (board->getCurrentPrice().higer.value() < board->StockValue()) {
            StockPrice price = getMarketPriceHigher(board, board->StockValue(), 1);
            trader.buy(price, {getTradeAmount()}, board->id, market);
            return;
        } else {
            // StockValueより高い

            // 買い手がいない
            if (!board->getCurrentPrice().lower) {
                StockPrice price = getMarketPriceLower(board, board->StockValue(), 1);
                if (price <= board->StockValue()) {
                    return;
                }
                auto count_tmp = getTradeAmount();
                auto count =
                    StockCount{trader.stock[stock] >= count_tmp ? count_tmp : trader.stock[stock].to_StockCount()};
                trader.sell(price, count, board->id, market);
                return;
            }
        }
    }
    if (board->getCurrentPrice().lower) {
        if (board->getCurrentPrice().lower.value() > board->StockValue()) {
            //  std::cout << "high\n";
            StockPrice price = getMarketPriceLower(board, board->StockValue(), 1);
            auto count_tmp = getTradeAmount();
            auto count = StockCount{trader.stock[stock] >= count_tmp ? count_tmp : trader.stock[stock].to_StockCount()};
            trader.sell(price, count, board->id, market);
            return;
        } else {
            // 売り手がいない
            if (!board->getCurrentPrice().higer) {
                StockPrice price = getMarketPriceHigher(board, board->StockValue(), 1);
                if (price >= board->StockValue()) {
                    return;
                }
                trader.buy(price, {getTradeAmount()}, board->id, market);
                return;
            }
        }
    }
    // std::cout << "nothing\n";
}

}  // namespace bot::trader
