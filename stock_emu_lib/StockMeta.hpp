#pragma once

#ifndef INCLUDE_STOCKMETA_HPP_
#include <stock_emu_lib/Stock.hpp>
#include <stock_emu_lib/util/MultiVector.hpp>

struct StockMeta {
    std::tuple<double, double> price_standard_base;
};

class StockMetaList {
public:
    const auto& getRate() const noexcept {
        return price_standard_rate;
    }

    void add(StockMeta&& add_meta) {
        meta.push_back(std::move(add_meta));
        price_standard_rate.push(0, 0);
    }

private:
    std::vector<StockMeta> meta;
    util::MultiVector<double, double> price_standard_rate;
};

#endif  // INCLUDE_STOCKMETA_HPP_
