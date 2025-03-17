#pragma once
#ifndef INCLUDE_MONEY_HPP_
#define INCLUDE_MONEY_HPP_
#include <format>

using Money_data_t = int;

class Price {
public:
    constexpr Price(Money_data_t value_) : value(value_) {}

    constexpr Price operator*(Money_data_t amount) const noexcept {
        return Price(value * amount);
    }

    constexpr auto operator<=>(const Price& rhs) const noexcept {
        return this->value <=> rhs.value;
    }

    constexpr bool operator==(const Price& r) const noexcept {
        return this->value == r.value;
    }

    constexpr auto getValue() const noexcept {
        return value;
    }

private:
    Money_data_t value;
};

class Money {
public:
    constexpr Money(Money_data_t value_) : value(value_) {}

    constexpr Money() : value(0) {};

    // moveする場合、全部受け渡し先に移動
    constexpr Money(Money&& other) : value(0) {
        other.move(other.as_Price()).to(*this);
    }

    constexpr Money_data_t getValue() const noexcept {
        return value;
    }

    constexpr Price as_Price() const {
        return Price(value);
    }

    constexpr auto operator<=>(const Money& r) const noexcept {
        return this->value <=> r.value;
    }

    constexpr Money devide(Price price) {
        Money result{0};
        this->move(price).to(result);
        return result;
    }

    // ムーブセマンティクスを使わないで複製されるのは事故の元
    Money(const Money&) = delete;
    constexpr auto operator=(const Money&) = delete;

private:
    Money_data_t value;

    constexpr Money& operator+=(const Money& money) noexcept {
        this->value += money.value;
        return *this;
    }

    constexpr Money& operator-=(const Money& money) noexcept {
        this->value -= money.value;
        return *this;
    }

    struct money_move_helper {
        constexpr void to(Money& to) {
            from.value -= price.getValue();
            to.value += price.getValue();
        }

        constexpr money_move_helper(Price price_, Money& from_) : price(price_), from(from_) {}

    private:
        Price price;
        Money& from;
    };

public:
    money_move_helper move(Price price) {
        return {price, *this};
    }
};

template<>
struct std::formatter<Money> : std::formatter<Money_data_t> {
    auto format(const Money& c, std::format_context& ctx) const {
        return std::formatter<Money_data_t>::format(c.getValue(), ctx);
    }
};

#endif  // INCLUDE_MONEY_HPP
