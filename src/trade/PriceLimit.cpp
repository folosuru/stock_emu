#include "PriceLimit.hpp"

bool PriceLimit::is_withen_PriceLimit(StockPrice price) const noexcept {
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
        return price;
    }
    if (PriceLimit->PriceLimit_high < price) {
        return PriceLimit->PriceLimit_high;
    } else if (PriceLimit->PriceLimit_low > price) {
        return PriceLimit->PriceLimit_low;
    }
    return price;
}
