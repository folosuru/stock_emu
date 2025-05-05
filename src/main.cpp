#include <chrono>
#include <cstdio>
#include <fstream>
#include <memory>
#include <random>

#include "Stock.hpp"
#include "bot/trader/Trader.hpp"
#include "money.hpp"
#include "trade/Trade.hpp"

std::random_device seed_gen;
std::default_random_engine engine(seed_gen());

inline void printstat(StockMarket& market, int id) {
    market.get(id)->printAll();

    if (market.get(id)->getCurrentPrice().higer.has_value()) {
        std::printf("high: %d\n", market.get(id)->getCurrentPrice().higer->getValue());
    } else {
        std::printf("high: --\n");
    }

    if (market.get(id)->getCurrentPrice().lower.has_value()) {
        std::printf("low:  %d\n", market.get(id)->getCurrentPrice().lower->getValue());
    } else {
        std::printf("low:  --\n");
    }

    auto board = market.get(id);

    auto& lim = board->getPriceLimit().get();
    if (lim) {
        std::printf("lim: %d~%d\n", lim->PriceLimit_low.getValue(), lim->PriceLimit_high.getValue());
    } else {
        std::printf("lim: -\n");
    }
    std::printf("\n");
}

int main() {
    auto market = std::make_shared<StockMarket>();
    auto id = market->add({700});
    auto id2 = market->add({500});
    auto id3 = market->add({1700});

    auto trader = Trader(1, 10000000, Trader::StockData_t::create({{id, 10000}, {id2, 10000}, {id3, 10000}}));
    auto trader2 = Trader(2, 10000000, Trader::StockData_t::create({{id, 10000}, {id2, 10000}, {id3, 10000}}));
    auto trader3 = Trader(3, 10000000, Trader::StockData_t::create({{id, 10000}, {id2, 10000}, {id3, 10000}}));

    std::uniform_int_distribution<> stock_select(0, market->count() - 1);

    auto board = (*market)[id];
    auto board2 = (*market)[id2];

    auto user = Trader(2, 5000, Trader::StockData_t::create({{id, 10000}, {id2, 100000}}));

    trader.buy({298}, {50}, id, market);
    trader.sell({302}, {50}, id, market);
    trader.buy({302}, {25}, id, market);
    // trader.buy({302}, {50}, id, market);

    printstat(*market, id);

    trader.printStat();
    user.printStat();
    auto bot = bot::trader::Random();
    auto bot2 = bot::trader::Fundamental();
    auto bot3 = bot::trader::Fundamental();

    if (market->get(id)->getCurrentPrice().higer.has_value()) {
        std::printf("high: %d\n", market->get(id)->getCurrentPrice().higer->getValue());
    } else {
        std::printf("high: --\n");
    }

    if (market->get(id)->getCurrentPrice().lower.has_value()) {
        std::printf("low:  %d\n", market->get(id)->getCurrentPrice().lower->getValue());
    } else {
        std::printf("low:  --\n");
    }

    std::printf("latest:  %d\n", market->get(id)->getCurrentPrice().latest.getValue());
    market->get(id)->printAll();

    //
    // return 0;

    std::ofstream file("out.csv");
    for (int loop = 0; loop < 10; loop++) {
        // auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            bot.Trade(trader, market);
            bot2.Trade(trader2, market);
            bot3.Trade(trader3, market);
        }

        /*
        auto end = std::chrono::high_resolution_clock::now();  // 計測終了
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Execution time: " << duration.count() << " ms\n";
        */
        file << board->getCurrentPrice().latest.getValue() << "," << board2->getCurrentPrice().latest.getValue() << ","
             << market->get(id3)->getCurrentPrice().latest.getValue() << ",";
        market->tick();
        if (loop % 50 == 49) {
            const auto select_id = stock_select(engine);
            std::cout << "selected:" << select_id << "\n";
            std::normal_distribution dist(0.0, 1.0);
            auto rand_board = market->get(select_id);
            rand_board->updateStockValue({rand_board->StockValue().getValue() * std::pow(2.0, dist(engine))}, market);
        }
        board->printAll();

        if (market->get(id)->getCurrentPrice().higer.has_value()) {
            std::printf("high: %d\n", market->get(id)->getCurrentPrice().higer->getValue());
        } else {
            std::printf("high: --\n");
        }

        if (market->get(id)->getCurrentPrice().lower.has_value()) {
            std::printf("low:  %d\n", market->get(id)->getCurrentPrice().lower->getValue());
        } else {
            std::printf("low:  --\n");
        }
        // trader.printStat();
        // trader2.printStat();
        // board->printAll();
    }

    file.close();

    return 0;
}
