#include "./Trader.hpp"

#include <random>
#include <stock_emu_lib/Stock.hpp>
#include <stock_emu_lib/money.hpp>
#include <stock_emu_lib/trade/Trade.hpp>

namespace bot::trader {

namespace {
std::random_device seed_gen;
// std::default_random_engine engine(seed_gen());
std::default_random_engine engine{64};
std::uniform_int_distribution<> action_dist(0, 2);
std::uniform_real_distribution<> amount_dist(0.0, 1.0);

inline StockPrice getMarketPriceHigher(const std::shared_ptr<TradeBoard> &board, StockPrice default_price,
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

inline StockPrice getMarketPriceLower(const std::shared_ptr<TradeBoard> &board, StockPrice default_price,
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
    std::uniform_int_distribution<StockId> stock_select(0, market->count() - 1);
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
        default: {
            // unreachable
        }
    }
}

void Fundamental::Trade(Trader &trader, StockMarketRef market) {
    std::discrete_distribution<StockId> stock_select(market->get_value_dondake_hanareteru().begin(),
                                                     market->get_value_dondake_hanareteru().end());
    StockId stock = stock_select(engine);
    auto board = market->get(stock);

    // std::cout << "value:" << board->StockValue().getValue() << ", price:" <<
    // board->getCurrentPrice().latest.getValue()
    //          << "\n";
    //
    const auto &datalist = market->getDatalist();
    auto data = datalist.getRate().getRow(stock);
    auto value_data1 = std::get<0>(data);
}

}  // namespace bot::trader
