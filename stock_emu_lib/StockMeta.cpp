#include "StockMeta.hpp"

#include <cstddef>
#include <tuple>
#include <utility>

#include "stock_emu_lib/Stock.hpp"
#include "stock_emu_lib/util/MultiVector.hpp"

namespace {
template<size_t index, class... params>
void update_price_foreach(util::MultiVector<params...>& vec, const std::tuple<params...>& base, StockId id,
                          StockPrice new_price) {
    if constexpr (index < sizeof...(params)) {
        vec.template getItem<index>(id) = std::get<index>(base) / new_price.getValue();
        update_price_foreach<index + 1>(vec, base, id, new_price);
    }
}
}

void StockMetaList::update(size_t index, StockPrice new_base_price) {
    update_price_foreach<0>(this->price_standard_rate, this->meta[index].price_standard_base, index, new_base_price);
}
