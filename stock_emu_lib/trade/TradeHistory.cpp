#include "TradeHistory.hpp"

#include <optional>
#include <stock_emu_lib/Stock.hpp>
#include <stock_emu_lib/trade/Trade.hpp>

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
        return;
    }
    if (board.getSellBoard().OrderExists(price)) {  // 2の一部
        return;
    }

    auto iter = board.getSellBoard().order_list.begin();
    while (board.getSellBoard().order_list.end() != iter) {
        if (iter->second.is_empty()) {
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
        return;
    }
    if (board.getBuyBoard().OrderExists(price)) {  // 3の一部
        return;
    }

    auto iter = board.getBuyBoard().order_list.rbegin();
    while (board.getBuyBoard().order_list.rend() != iter) {
        if (iter->second.is_empty()) {
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

    if (currentTick.start.getValue() == TickHistory::nothing) {
        currentTick.start = price;
    }

    currentTick.end = price;

    if (currentTick.high < price) {
        currentTick.high = price;
    }
    if (price < currentTick.low) currentTick.low = price;
}

const TickHistory TradeBoardHistory::tick() {
    const auto result = currentTick;
    tick_history.push(result.start, result.end, result.high, result.low);
    currentTick = TickHistory{};
    return result;
}
