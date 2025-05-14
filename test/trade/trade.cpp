#include <gtest/gtest.h>

#include <stock_emu_lib/trade/Trade.hpp>

#include "stock_emu_lib/Stock.hpp"

TEST(TradeBoard, SellCurrentPrice) {
    auto market = std::make_shared<StockMarket>();
    auto id = market->add({400});

    auto trader = Trader(1, 10000000,
                         Trader::StockData_t::create({
                             {id, 10000},
                         }));
    auto board = market->get(id);

    trader.sell({100}, {50}, id, market);
    {
        EXPECT_FALSE(board->getBuyBoard().OrderExists({100}));
        EXPECT_TRUE(board->getSellBoard().OrderExists({100}));
        auto current = board->getCurrentPrice();
        EXPECT_EQ(current.latest.getValue(), 0);

        EXPECT_EQ(current.higer.has_value(), true);
        EXPECT_EQ(current.higer->getValue(), 100);
        EXPECT_EQ(current.lower.has_value(), false);
    }

    trader.buy({100}, {25}, id, market);
    {
        EXPECT_FALSE(board->getBuyBoard().OrderExists({100}));
        EXPECT_TRUE(board->getSellBoard().OrderExists({100}));

        auto current = board->getCurrentPrice();
        EXPECT_EQ(current.latest.getValue(), 100);
        EXPECT_EQ(current.higer.has_value(), true);
        EXPECT_EQ(current.higer->getValue(), 100);
        EXPECT_EQ(current.lower.has_value(), false);
    }

    trader.buy({100}, {25}, id, market);
    {
        auto current = board->getCurrentPrice();
        EXPECT_FALSE(board->getBuyBoard().OrderExists({100}));
        EXPECT_FALSE(board->getSellBoard().OrderExists({100}));
        EXPECT_EQ(current.latest.getValue(), 100);
        EXPECT_EQ(current.lower.has_value(), false);
        EXPECT_EQ(current.higer.has_value(), false);
    }
}

TEST(TradeBoard, BuyCurrentPrice) {
    auto market = std::make_shared<StockMarket>();
    auto id = market->add({400});

    auto trader = Trader(1, 10000000,
                         Trader::StockData_t::create({
                             {id, 10000},
                         }));
    auto board = market->get(id);

    trader.buy({100}, {50}, id, market);
    {
        EXPECT_TRUE(board->getBuyBoard().OrderExists({100}));
        EXPECT_FALSE(board->getSellBoard().OrderExists({100}));
        auto current = board->getCurrentPrice();
        EXPECT_EQ(current.latest.getValue(), 0);

        EXPECT_EQ(current.lower.has_value(), true);
        EXPECT_EQ(current.lower->getValue(), 100);
        EXPECT_EQ(current.higer.has_value(), false);
    }

    trader.sell({100}, {25}, id, market);
    {
        EXPECT_TRUE(board->getBuyBoard().OrderExists({100}));
        EXPECT_FALSE(board->getSellBoard().OrderExists({100}));

        auto current = board->getCurrentPrice();
        EXPECT_EQ(current.latest.getValue(), 100);
        EXPECT_EQ(current.lower.has_value(), true);
        EXPECT_EQ(current.lower->getValue(), 100);
        EXPECT_EQ(current.higer.has_value(), false);
    }

    trader.sell({100}, {25}, id, market);
    {
        auto current = board->getCurrentPrice();
        EXPECT_FALSE(board->getBuyBoard().OrderExists({100}));
        EXPECT_FALSE(board->getSellBoard().OrderExists({100}));
        EXPECT_EQ(current.latest.getValue(), 100);
        EXPECT_EQ(current.lower.has_value(), false);
        EXPECT_EQ(current.higer.has_value(), false);
    }
}
