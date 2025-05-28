#pragma once
#ifndef INCLUDE_TRADE_TRADER_HPP_
#define INCLUDE_TRADE_TRADER_HPP_

#include <format>
#include <iostream>
#include <map>
#include <stock_emu_lib/Stock.hpp>
#include <stock_emu_lib/money.hpp>

class StockMarket;
using StockMarketRef = std::shared_ptr<StockMarket>;

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

#endif  // !INCLUDE_TRADE_TRADER_HPP_
