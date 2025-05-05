#include "PriceLimit.hpp"

#include <algorithm>

#include "Stock.hpp"
#include "money.hpp"

bool PriceLimit::is_withen_PriceLimit(StockPrice price) const noexcept {
    if (price.getValue() <= 0) {
        return false;
    }
    if (!PriceLimit) {
        return true;
    }
    if (PriceLimit->PriceLimit_high < price) {
        return false;
    }
    if (PriceLimit->PriceLimit_low > price) {
        return false;
    }
    return true;
}

StockPrice PriceLimit::to_withen_PriceLimit(StockPrice price) const noexcept {
    if (!PriceLimit) {
        if (price.getValue() <= 0) {
            return {1};
        }
        return price;
    }
    if (PriceLimit->PriceLimit_high < price) {
        return PriceLimit->PriceLimit_high;
    } else if (price < PriceLimit->PriceLimit_low) {
        return PriceLimit->PriceLimit_low;
    }
    return price;
}

void PriceLimit::update_PriceLimit(StockPrice price) noexcept {
    const double PriceLimit_width = price.getValue() * width_rate;
    const double new_higher = price.getValue() + PriceLimit_width;
    const double new_lower = price.getValue() - PriceLimit_width;

    const StockPrice new_higher_fixed = {
        std::max(static_cast<Money_data_t>(new_higher), price.getValue() + lower_limit_width)};
    const StockPrice new_lower_fixed = {
        std::max(1, std::min(static_cast<Money_data_t>(new_lower), price.getValue() - lower_limit_width))};

    PriceLimit = {new_higher_fixed, new_lower_fixed};
}
