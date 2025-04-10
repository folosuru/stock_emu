#pragma once
#include <limits>
#include <utility>

#include "money.hpp"
#ifndef INCLUDE_TRADE_HPP_
#define INCLUDE_TRADE_HPP_

#include <Stock.hpp>
#include <cstdio>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "util/RingQueue.hpp"

struct TradeBoard;
struct Trader;
class StockMarket;
using StockMarketRef = std::shared_ptr<StockMarket>;
using Tick_t = int;

struct TradeOrder {
    StockPrice price;
    StockCount amount;
    StockId id;
    Trader& trader;
};

struct TradeRequest {
    StockCount amount;
    Trader& trader;
};

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

struct Trader {
    StockId id;
    Money money;

    struct StockData_t : public std::map<StockId, StockHoldingCount> {
        static StockData_t create(const std::vector<std::pair<StockId, StockCount_data_t>>& data);
    };

    StockData_t stock;

    void printStat() {
        // std::printf("user %d\n  money: %d\n stock: %d\n", id, money.getValue(), stock[1]);
        std::cout << std::format("id:{}\n  money: {}\n  stock:\n", id, money);
        for (const auto& i : stock) {
            std::cout << std::format("    {}: {}\n", i.first, i.second);
        }
    }

    Trader(StockId id_, Money&& money_, StockData_t&& stock_ = {})
        : id(id_), money(std::move(money_)), stock(std::move(stock_)) {}

    void sell(StockPrice price, StockCount amount, StockId id, StockMarketRef ref);
    void buy(StockPrice price, StockCount amount, StockId id, StockMarketRef ref);
};

struct SellTradeRequest {
    StockHoldingCount amount;
    Trader& trader;
    StockId id;
    SellTradeRequest** refer = nullptr;

    SellTradeRequest(StockHoldingCount&& amount_, Trader& trader_, StockId id_)
        : amount(std::move(amount_)), trader(trader_), id(id_) {}

    void reject() {
        amount.move(amount.to_StockCount()).to(trader.stock[id]);
    }
};

struct BuyTradeRequest {
    StockCount amount;
    Trader& trader;
    Money money;
    BuyTradeRequest** refer = nullptr;

    BuyTradeRequest(StockCount amount_, Trader& trader_, Money&& money)
        : amount(amount_), trader(trader_), money(std::move(money)) {}

    void reject() {
        money.move(money.as_Price()).to(trader.money);
        this->amount = 0;
    }

    BuyTradeRequest() = delete;
};

struct TradeBoard {
    StockId id;
    std::deque<TradeHistory> history{{{300}, {100}}};
    std::deque<TickHistory> tick_history{};

    TradeBoard(StockId id_, StockMarket& market_, StockPrice value = {300})
        : id(id_), market_ref(market_), stock_value(value) {
        CurrentStockPrice.higer = {302};
        CurrentStockPrice.lower = {298};
    }

    bool LimitOrder_Sell(StockPrice, StockCount, Trader&);
    bool LimitOrder_Buy(StockPrice, StockCount, Trader&);

    void MarketOrder_sell(StockCount count, Trader&);
    void MarketOrder_Buy(StockCount count, Trader&);

    void printAll() {
        std::printf("%10s|price|\n", " ");
        for (auto i = sell.order_list.rbegin(); i != sell.order_list.rend(); ++i) {
            const auto& price = i->first;
            StockCount amount_sum = 0;
            for (const auto& j : i->second) {
                amount_sum += j->amount.to_StockCount();
            }
            if (amount_sum == 0) continue;
            std::cout << std::format("{: >10} {: ^5} \n", amount_sum, price.getValue());
        }
        for (auto i = buy.order_list.rbegin(); i != buy.order_list.rend(); ++i) {
            const auto& price = i->first;
            StockCount amount_sum = 0;
            for (const auto& j : i->second) {
                amount_sum += j->amount;
            }
            if (amount_sum == 0) continue;
            std::cout << std::format("{:10} {: ^5} {: >10}\n", "", price.getValue(), amount_sum);
        }
    }

    auto& getCurrentPrice() const {
        return CurrentStockPrice;
    }

    const auto& getBuyBoard() const {
        return buy;
    }

    const auto& getSellBoard() const {
        return sell;
    }

    const auto& StockValue() const {
        return stock_value;
    }

    void updateStockValue(StockPrice price, StockMarketRef market);

    void tick() noexcept;

    StockPrice to_withen_PriceLimit(StockPrice pric) const noexcept;
    bool is_withen_PriceLimit(StockPrice price) const noexcept;

private:
    template<class Request_t, class limit_destructor_t>
    struct TradeRequestBoard {
        std::map<StockPrice, std::deque<Request_t*>> order_list = {};
        util::RingQueue<std::deque<Request_t>, 24, limit_destructor_t> limit_queue;

        template<class... Args>
        void add(int expire, StockPrice price, Args&&... arg) {
            limit_queue.get(expire).emplace_back(std::forward<Args>(arg)...);
            auto* elem = &limit_queue.get(expire).back();

            order_list[price].emplace_back(elem);
            auto* ref_ptr = &order_list[price].back();
            elem->refer = ref_ptr;
        }
    };

    struct buy_limit_destruct {
        static void run(std::deque<BuyTradeRequest>&);
    };

    struct sell_limit_destruct {
        static void run(std::deque<SellTradeRequest>&, StockId);
    };

    TradeRequestBoard<BuyTradeRequest, buy_limit_destruct> buy;
    TradeRequestBoard<SellTradeRequest, sell_limit_destruct> sell;

    struct {
        std::optional<StockPrice> higer;
        std::optional<StockPrice> lower;
        StockPrice latest;
    } CurrentStockPrice;

    struct PriceLimit_data {
        StockPrice PriceLimit_high;
        StockPrice PriceLimit_low;
    };

    std::optional<PriceLimit_data> PriceLimit = std::nullopt;

    StockMarket& market_ref;

    StockPrice stock_value = {300};

    void update_current_high();
    void update_current_low();

    void update_current_high(StockPrice price);
    void update_current_low(StockPrice price);

    void update_current_high_add(StockPrice price);
    void update_current_low_add(StockPrice price);

    void update_current_latest(StockPrice price);
    void update_history(StockPrice price, StockCount count);
};

class StockMarket {
public:
    std::shared_ptr<TradeBoard> get(StockId id) const {
        return boards[id];
    }

    std::shared_ptr<TradeBoard> operator[](StockId id) const {
        return get(id);
    }

    StockId add(StockPrice value = {300}) {
        auto new_id = static_cast<StockId>(boards.size());
        value_dondake_hanareteru.emplace_back();
        boards.emplace_back(new TradeBoard{new_id, *this, value});
        return new_id;
    }

    StockId count() const noexcept {
        return boards.size();
    }

    void updatePricePerValue(const TradeBoard&);

    /**
     * 現在の値段が価値とどのくらい離れているかのvectorを返す
     * 気が向いたらいい名前を考える
     */
    const auto& get_value_dondake_hanareteru() const {
        return value_dondake_hanareteru;
    }

    void tick() {
        for (auto& i : boards) {
            i->tick();
        }
    }

private:
    std::vector<std::shared_ptr<TradeBoard>> boards;

    std::vector<double> value_dondake_hanareteru;
};

inline Trader::StockData_t Trader::StockData_t::create(const std::vector<std::pair<StockId, StockCount_data_t>>& data) {
    Trader::StockData_t result;
    for (const auto& i : data) {
        result.emplace(i.first, StockHoldingCount{i.second});
    }
    return result;
}

#endif  // INCLUDE_TRADE_HPP_
