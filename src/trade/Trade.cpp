#include "Trade.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

#include "Stock.hpp"

void Trader::sell(StockPrice price, StockCount amount, StockId id, StockMarketRef ref) {
    (*ref)[id]->LimitOrder_Sell(price, amount, *this);
}

void Trader::buy(StockPrice price, StockCount amount, StockId id, StockMarketRef ref) {
    (*ref)[id]->LimitOrder_Buy(price, amount, *this);
}

bool TradeBoard::LimitOrder_Sell(StockPrice price, StockCount count, Trader& trader) {
    {
        if (trader.stock[id] < count) {  // 空売りは面倒なことになる
            return false;
        }
        if (!is_withen_PriceLimit(price)) return false;

        StockHoldingCount order_stocks = trader.stock[id].devide(count);
        auto& buyer_queue = buy.order_list[price];

        while (!buyer_queue.empty()) {
            auto* buy_order = buyer_queue.front();
            if (buy_order == nullptr) {
                buyer_queue.pop_front();
                continue;
            }
            auto& buyer_stock = buy_order->trader.stock[id];

            if (buy_order->amount < order_stocks) {
                buy_order->money.move(buy_order->money.as_Price()).to(trader.money);
                order_stocks.move(buy_order->amount).to(buyer_stock);
                buyer_queue.pop_front();

            } else if (buy_order->amount == order_stocks) {
                buy_order->money.move(buy_order->money.as_Price()).to(trader.money);
                order_stocks.move(buy_order->amount).to(buyer_stock);
                buyer_queue.pop_front();
                break;
            } else {
                buy_order->money.move(price * order_stocks.to_StockCount()).to(trader.money);
                order_stocks.move(order_stocks.to_StockCount()).to(buyer_stock);
                break;
            }
        }
        const auto sold_stock = count - order_stocks.to_StockCount();

        if (sold_stock != 0) {
            this->history.push_back({sold_stock, price});
            update_current_latest(price);
            update_current_low(price);
        }

        if (order_stocks != 0) {  // 売れ残りがある => order.priceでの買い手がいない
            sell.add(3, price, std::move(order_stocks), trader, id);
            update_current_high_add(price);
        }
    }
    return true;
}

bool TradeBoard::LimitOrder_Buy(StockPrice price, StockCount count, Trader& trader) {
    auto& seller_queue = sell.order_list[price];

    auto buy_price = price * count;

    if (trader.money.as_Price() < buy_price) {  // 借金もよくない
        return false;
    }

    if (!is_withen_PriceLimit(price)) return false;

    Money paid_money = trader.money.devide(buy_price);
    StockCount current_amount = count;
    auto& trader_stock = trader.stock[id];

    while (true) {
        if (seller_queue.empty() || current_amount == 0) {
            break;
        }

        auto* sell_order = seller_queue.front();
        if (sell_order == nullptr) {
            seller_queue.pop_front();
            continue;
        }
        auto& seller_stock = sell_order->amount;
        if (seller_stock == 0) {
            seller_queue.pop_front();
            continue;
        }
        auto& seller_money = sell_order->trader.money;

        StockCount trade_stock_count = std::min(current_amount, seller_stock.to_StockCount());

        current_amount -= trade_stock_count;
        paid_money.move(price * trade_stock_count).to(seller_money);
        sell_order->amount.move(trade_stock_count).to(trader_stock);
    }
    auto sold_stock = count - current_amount;
    if (sold_stock != 0) {
        update_current_high(price);
        update_current_latest(price);
    }
    if (current_amount != 0) {
        buy.add(3, price, current_amount, trader, std::move(paid_money));
        update_current_low_add(price);
    }
    return true;
}

void TradeBoard::update_history(StockPrice price, StockCount count) {
    this->history.push_back({count, price});
    auto& tick_history = this->tick_history.back();
    if (tick_history.start.getValue() == std::numeric_limits<StockCount_data_t>::min()) {
        tick_history.start = price;
    }
    tick_history.end = price;
}

void TradeBoard::tick() noexcept {
    this->tick_history.emplace_back();
    market_ref.updatePricePerValue(*this);
}

void StockMarket::updatePricePerValue(const TradeBoard& ref) {
    this->value_dondake_hanareteru.at(ref.id) =
        std::abs(ref.getCurrentPrice().latest.getValue() - ref.StockValue().getValue());
}
