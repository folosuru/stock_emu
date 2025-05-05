#pragma once
#ifndef INCLUDE_STOCK_HPP_
#define INCLUDE_STOCK_HPP_
#include <format>
#include <iostream>
#include <money.hpp>
#include <type_traits>

struct StockCount;
using StockId = int;
using StockCount_data_t = int;

class StockPrice {
public:
    constexpr StockPrice(Price price) : price(price) {}

    constexpr StockPrice() : price(0) {}

    constexpr Price operator*(int amount) const noexcept {
        return price * amount;
    }

    constexpr Price operator*(double amount) const noexcept {
        return price * amount;
    }

    constexpr auto operator<=>(const StockPrice& rhs) const {
        return this->price <=> rhs.price;
    }

    constexpr bool operator==(const StockPrice& r) const {
        return this->price == r.price;
    }

    constexpr auto& operator=(const StockPrice& r) {
        this->price = r.price;
        return *this;
    }

    constexpr int getValue() const noexcept {
        return price.getValue();
    }

private:
    Price price;
};

struct StockCount {
    StockCount_data_t value;

    constexpr StockCount(StockCount_data_t v) : value(v) {}

    constexpr StockCount() : value(0) {}

    constexpr auto operator+(const StockCount& left) const {
        return StockCount(this->value + left.value);
    }

    constexpr auto operator-(const StockCount& left) const {
        return StockCount(this->value - left.value);
    }

    template<class T>
    constexpr auto operator*(T mlu) const {
        static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "operator* only accept number");
        return StockCount{static_cast<int>(value * mlu)};
    }

    constexpr auto& operator+=(const StockCount& left) {
        this->value += left.value;
        return *this;
    }

    constexpr auto& operator-=(const StockCount& left) {
        this->value -= left.value;
        return *this;
    }

    constexpr auto operator<=>(const StockCount&) const = default;

    constexpr auto operator<=>(StockCount_data_t v) const {
        return value <=> v;
    }
};

class StockHoldingCount {
public:
    constexpr StockHoldingCount(StockCount_data_t count) : value(count) {}

    constexpr StockHoldingCount() : value(0) {}

    constexpr StockCount to_StockCount() const {
        return StockCount(value);
    };

    constexpr StockCount_data_t getValue() const {
        return value;
    }

    constexpr ~StockHoldingCount() {
        if (std::is_constant_evaluated()) {
            static_assert(true, "cannot destruct stock");
        } else {
        }
    }

    StockHoldingCount(const StockHoldingCount&) = delete;
    auto operator=(const StockHoldingCount&) = delete;

    StockHoldingCount(StockHoldingCount&& other) {
        other.move(other.to_StockCount()).to(*this);
    }

    StockHoldingCount devide(StockCount count) {
        StockHoldingCount result{0};
        this->move(count).to(result);
        return result;
    }

    constexpr auto operator<=>(StockCount v) const {
        return value <=> v.value;
    }

    constexpr bool operator==(const StockCount& v) const {
        return value == v.value;
    }

private:
    StockCount_data_t value = 0;

    struct move_to_t {
        constexpr inline void to(StockHoldingCount& to) {
            to.value += amount.value;
            from.value -= amount.value;
        }

        constexpr move_to_t(StockHoldingCount& from_, StockCount amount_) : from(from_), amount(amount_) {}

    private:
        StockHoldingCount& from;
        const StockCount amount;
    };

public:
    constexpr move_to_t move(StockCount amount) {
        return move_to_t(*this, amount);
    }
};

static_assert([]() {
    StockHoldingCount a{10}, b{30};
    a.move(a.to_StockCount()).to(b);
    return (a.getValue() == 0 && b.getValue() == 40);
}());

inline StockCount operator/(Money& money, const StockPrice& price) {
    std::cout << money.getValue() << "/" << price.getValue() << std::endl;
    return {money.getValue() / price.getValue()};
}

inline Price operator*(const StockPrice& price, const StockCount& count) {
    return Price{price.getValue() * count.value};
}

inline bool operator<(const StockPrice& l, const StockPrice& r) {
    return l.getValue() < r.getValue();
}

inline bool operator>(const StockPrice& l, const StockPrice& r) {
    return l.getValue() > r.getValue();
}

template<>
struct std::formatter<StockCount> : std::formatter<int> {
    auto format(StockCount c, std::format_context& ctx) const {
        return std::formatter<int>::format(c.value, ctx);
    }
};

template<>
struct std::formatter<StockHoldingCount> : std::formatter<int> {
    auto format(const StockHoldingCount& c, std::format_context& ctx) const {
        return std::formatter<int>::format(c.getValue(), ctx);
    }
};

template<>
struct std::formatter<StockPrice> : std::formatter<int> {
    auto format(StockPrice c, std::format_context& ctx) const {
        return std::formatter<int>::format(c.getValue(), ctx);
    }
};
#endif
