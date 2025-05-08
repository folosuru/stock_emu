#include "TradeHistory.hpp"

#include <cstdio>
#include <optional>
#include <trade/Trade.hpp>

#include "Stock.hpp"

void TradeBoardHistory::update_sell_board_add(StockPrice price) {
    if (!CurrentStockPrice.higer.has_value() || price < this->CurrentStockPrice.higer) {
        CurrentStockPrice.higer = price;
    }
}

void TradeBoardHistory::update_buy_board_add(StockPrice price) {
    if (!CurrentStockPrice.lower.has_value() || price > CurrentStockPrice.lower) {
        CurrentStockPrice.lower = price;
    }
}

void TradeBoardHistory::update_current_high(StockPrice price, const TradeBoard& board) {
    /*
     * 起こりうるケースは：
     *
     * | sell  |  buy |
     * | (1)   |      |
     * | (2)   |      |
     * |       |  (3) |
     * |       |  (4) |
     *
     * (1) が約定
     * (2) が一部約定
     * (2) が全部約定
     * (3) が一部約定
     * (3) が全部約定
     * (4) が約定
     *
     * に分類される。このうち、処理する必要があるのは2の全部約定だけ
     */
    if (this->CurrentStockPrice.higer != price) {  // is (1), (3), (4)
        std::printf("not current: return\n");
        return;
    }
    if (board.getSellBoard().OrderExists(price)) {  // 2の一部

        std::printf("seller: order exists (%d)\n", price.getValue());
        return;
    }

    auto iter = board.getSellBoard().order_list.begin();
    while (board.getSellBoard().order_list.end() != iter) {
        if (iter->second.empty()) {
            iter++;
            continue;
        }
        break;
    }
    if (iter != board.getSellBoard().order_list.end()) {
        this->CurrentStockPrice.higer = iter->first;
    } else {
        this->CurrentStockPrice.higer = std::nullopt;
    }
}

void TradeBoardHistory::update_current_low(StockPrice price, const TradeBoard& board) {
    if (this->CurrentStockPrice.lower != price) {  // is (1), (2), (4)
        std::printf("not current: return\n");
        return;
    }
    if (board.getBuyBoard().OrderExists(price)) {  // 3の一部
        std::printf("buyer: order exists (%d)\n", price.getValue());
        return;
    }

    auto iter = board.getBuyBoard().order_list.rbegin();
    while (board.getBuyBoard().order_list.rend() != iter) {
        if (iter->second.empty()) {
            iter++;
            continue;
        }
        break;
    }

    if (iter != board.getBuyBoard().order_list.rend()) {
        this->CurrentStockPrice.lower = iter->first;
    } else {
        this->CurrentStockPrice.lower = std::nullopt;
    }
}

void TradeBoardHistory::update_history(StockPrice price, StockCount count) {
    CurrentStockPrice.latest = price;
    this->history.push_back({count, price});

    auto& current_history = tick_history.back();
    if (current_history.start.getValue() == TickHistory::nothing) {
        current_history.start = price;
    }

    current_history.end = price;

    if (current_history.high < price) {
        current_history.high = price;
    }
    if (current_history.low > price) {
        current_history.low = price;
    }
}

const TickHistory& TradeBoardHistory::tick() {
    const auto& result = tick_history.back();
    tick_history.emplace_back();
    return result;
}
