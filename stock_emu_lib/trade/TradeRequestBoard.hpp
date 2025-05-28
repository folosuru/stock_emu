#pragma once
#ifndef INCLUDE_TRADE_TRADEREQUESTBOARD_HPP_
#define INCLUDE_TRADE_TRADEREQUESTBOARD_HPP_
#include <map>
#include <stock_emu_lib/Stock.hpp>
#include <stock_emu_lib/trade/Trader.hpp>
#include <stock_emu_lib/util/RingQueue.hpp>

template<class T>
concept TradeRequest = requires(T& t) { t.amount == 0; };

namespace Trade {

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
            return false;
        }
        return true;
    }
};
}
#endif  // !INCLUDE_TRADE_TRADEREQUESTBOARD_HPP_
