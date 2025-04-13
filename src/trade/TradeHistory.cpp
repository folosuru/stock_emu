#include "TradeHistory.hpp"

void TradeBoardHistory::update_current_high_add(StockPrice price) {
    if (price < this->CurrentStockPrice.higer) {
        CurrentStockPrice.higer = price;
    }
}

void TradeBoardHistory::update_current_low_add(StockPrice price) {
    if (price > CurrentStockPrice.lower) {
        CurrentStockPrice.lower = price;
    }
}

void TradeBoardHistory::update_current_latest(StockPrice price) {
    CurrentStockPrice.latest = price;
}

void TradeBoardHistory::update_current_high(StockPrice price) {
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
    if (!this->sell.order_list[price].empty()) {  // 2の一部
        return;
    }

    auto iter = this->sell.order_list.begin();
    while (this->sell.order_list.end() != iter) {
        if (iter->second.empty()) {
            iter++;
            continue;
        }
        break;
    }

    this->CurrentStockPrice.higer = iter->first;
}

void TradeBoardHistory::update_current_low(StockPrice price) {
    if (this->CurrentStockPrice.lower != price) {  // is (1), (3), (4)
        return;
    }
    if (!this->sell.order_list[price].empty()) {  // 2の一部
        return;
    }

    auto iter = this->buy.order_list.rbegin();
    while (this->buy.order_list.rend() != iter) {
        if (iter->second.empty()) {
            iter++;
            continue;
        }
        break;
    }

    this->CurrentStockPrice.lower = iter->first;
}
