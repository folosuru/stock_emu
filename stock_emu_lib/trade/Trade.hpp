#pragma once
#include "stock_emu_lib/StockMeta.hpp"
#ifndef INCLUDE_TRADE_HPP_
#define INCLUDE_TRADE_HPP_

#include <deque>
#include <memory>
#include <utility>
#include <vector>
//
#include <stock_emu_lib/Stock.hpp>
#include <stock_emu_lib/money.hpp>
#include <stock_emu_lib/trade/PriceLimit.hpp>
#include <stock_emu_lib/util/RingQueue.hpp>

#include "TradeHistory.hpp"
#include "stock_emu_lib/trade/TradeRequestBoard.hpp"

struct TradeBoard;
struct Trader;
class StockMarket;
using StockMarketRef = std::shared_ptr<StockMarket>;
using Tick_t = int;

struct TradeBoard {
public:
    StockId id;

    constexpr static int expire_time = 4;

    TradeBoard(StockId id_, StockMarket& market_) : id(id_), market_ref(market_) {}

    bool LimitOrder_Sell(StockPrice, StockCount, Trader&);
    bool LimitOrder_Buy(StockPrice, StockCount, Trader&);

    void MarketOrder_sell(StockCount count, Trader&);
    void MarketOrder_Buy(StockCount count, Trader&);

    void printAll();

    constexpr const auto& getBuyBoard() const {
        return buy;
    }

    constexpr const auto& getSellBoard() const {
        return sell;
    }

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
    struct buy_limit_destruct {
        static void run(std::deque<Trade::BuyTradeRequest>&);
    };

    struct sell_limit_destruct {
        static void run(std::deque<Trade::SellTradeRequest>&);
    };

    Trade::TradeRequestBoard<Trade::BuyTradeRequest, buy_limit_destruct> buy;
    Trade::TradeRequestBoard<Trade::SellTradeRequest, sell_limit_destruct> sell;

    StockMarket& market_ref;

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

    StockId add(StockMeta&& meta) {
        auto new_id = static_cast<StockId>(boards.size());
        data_list.add(std::move(meta));
        boards.emplace_back(new TradeBoard{new_id, *this});
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
        return data_list.getRate().getColumn<0>();
    }

    auto& getDatalist() const noexcept {
        return data_list;
    }

    void tick() {
        for (auto& i : boards) {
            i->tick();
        }
    }

private:
    std::vector<std::shared_ptr<TradeBoard>> boards;
    StockMetaList data_list;
};

inline Trader::StockData_t Trader::StockData_t::create(const std::vector<std::pair<StockId, StockCount_data_t>>& data) {
    Trader::StockData_t result;
    for (const auto& i : data) {
        result.emplace(i.first, StockHoldingCount{i.second});
    }
    return result;
}

#endif  // INCLUDE_TRADE_HPP_
