#pragma once
#ifndef INCLUDE_TRADE_HPP_
#define INCLUDE_TRADE_HPP_

#include <array>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <utility>
#include <vector>
//
#include <stock_emu_lib/Stock.hpp>
#include <stock_emu_lib/money.hpp>
#include <stock_emu_lib/trade/PriceLimit.hpp>
#include <stock_emu_lib/util/RingQueue.hpp>

#include "TradeHistory.hpp"

struct TradeBoard;
struct Trader;
class StockMarket;
using StockMarketRef = std::shared_ptr<StockMarket>;
using Tick_t = int;

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

template<class T>
concept TradeRequest = requires(T& t) { t.amount == 0; };

struct SellTradeRequest {
    StockHoldingCount amount;
    Trader& trader;
    StockId id;
    SellTradeRequest** refer = nullptr;

    SellTradeRequest(StockHoldingCount&& amount_, Trader& trader_, StockId id_)
        : amount(std::move(amount_)), trader(trader_), id(id_) {}

    void reject() {
        amount.move(amount.to_StockCount()).to(trader.stock[id]);
        // *refer = nullptr;
    }
};

struct BuyTradeRequest {
    StockCount amount;
    Trader& trader;
    Money money;
    BuyTradeRequest** refer = nullptr;

    BuyTradeRequest(StockCount amount_, Trader& trader_, Money&& money_)
        : amount(amount_), trader(trader_), money(std::move(money_)) {}

    void reject() {
        money.move(money.as_Price()).to(trader.money);
        this->amount = 0;
        // *refer = nullptr;
    }

    BuyTradeRequest() = delete;
};

struct TradeBoard {
public:
    StockId id;

    constexpr static int expire_time = 4;

    TradeBoard(StockId id_, StockMarket& market_, StockPrice value = {300})
        : id(id_), market_ref(market_), stock_value(value) {}

    bool LimitOrder_Sell(StockPrice, StockCount, Trader&);
    bool LimitOrder_Buy(StockPrice, StockCount, Trader&);

    void MarketOrder_sell(StockCount count, Trader&);
    void MarketOrder_Buy(StockCount count, Trader&);

    void printAll() {
        constexpr int show_limit = 10;

        std::printf("%10s|price|\n", " ");

        struct count_price {
            StockPrice price;
            StockCount count;
        };

        std::array<count_price, show_limit> prices;
        size_t price_current_count = 0;

        for (auto i = sell.order_list.begin(); i != sell.order_list.end(); ++i) {
            const auto& price = i->first;
            StockCount amount_sum = 0;
            for (const auto& j : i->second) {
                amount_sum += j->amount.to_StockCount();
            }
            if (amount_sum == 0) continue;
            prices[price_current_count] = {price, amount_sum};
            price_current_count++;
            if (price_current_count == show_limit) break;
            // std::cout << std::format("{: >10} {: ^5} \n", amount_sum, price.getValue());
        }
        for (auto i = prices.rbegin(); i != prices.rend(); i++) {
            if (i->count.value == 0) {
                continue;
            }
            std::cout << std::format("{: >10} {: ^5} \n", i->count, i->price.getValue());
        }

        int count = 0;
        for (auto i = buy.order_list.rbegin(); i != buy.order_list.rend(); ++i, ++count) {
            if (count > show_limit) {
                break;
            }

            const auto& price = i->first;
            StockCount amount_sum = 0;
            for (const auto& j : i->second) {
                amount_sum += j->amount;
            }
            if (amount_sum == 0) continue;
            std::cout << std::format("{:10} {: ^5} {: >10}\n", "", price.getValue(), amount_sum);
        }
    }

    constexpr const auto& getBuyBoard() const {
        return buy;
    }

    constexpr const auto& getSellBoard() const {
        return sell;
    }

    constexpr const auto& StockValue() const {
        return stock_value;
    }

    void updateStockValue(StockPrice price, StockMarketRef market);

    void tick() noexcept;

    constexpr const auto& getHistory() const noexcept {
        return history;
    }

    constexpr const auto& getPriceLimit() const noexcept {
        return limit;
    }

    constexpr const auto& getCurrentPrice() const noexcept {
        return history.getCurrentPrice();
    }

private:
    template<TradeRequest Request_t, class limit_destructor_t>
    struct TradeRequestBoard {
        class RequestList_Wrapper : public std::deque<Request_t*> {
        public:
            bool is_empty() const {
                for (const auto& i : *this) {
                    if (i != nullptr) {
                        if (i->amount != 0) {
                            return false;
                        }
                    }
                }
                return true;
            }
        };

        std::map<StockPrice, RequestList_Wrapper> order_list = {};
        util::RingQueue<std::deque<Request_t>, 24, limit_destructor_t> limit_queue;

        template<class... Args>
        void add(size_t expire, StockPrice price, Args&&... arg) {
            limit_queue.get(expire).emplace_back(std::forward<Args>(arg)...);
            auto* elem = &limit_queue.get(expire).back();

            order_list[price].emplace_back(elem);
            auto* ref_ptr = &order_list[price].back();
            elem->refer = ref_ptr;
        }

        bool OrderExists(StockPrice price) const {
            if (const auto find = order_list.find(price); find == order_list.end()) {
                return false;
            }
            if (order_list.at(price).empty()) {
                return false;
            }
            if (order_list.at(price).is_empty()) {
                printf("???\n");
                return false;
            }
            return true;
        }
    };

    struct buy_limit_destruct {
        static void run(std::deque<BuyTradeRequest>&);
    };

    struct sell_limit_destruct {
        static void run(std::deque<SellTradeRequest>&);
    };

    TradeRequestBoard<BuyTradeRequest, buy_limit_destruct> buy;
    TradeRequestBoard<SellTradeRequest, sell_limit_destruct> sell;

    StockMarket& market_ref;
    StockPrice stock_value = {300};

    TradeBoardHistory history;
    PriceLimit limit;
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
