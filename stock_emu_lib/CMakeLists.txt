project(stock_emu_lib CXX)
set(CMAKE_CXX_STANDARD 20)
add_library(stock_emu_lib STATIC 
    trade/Trade.cpp
    trade/TradeHistory.cpp
    trade/PriceLimit.cpp
    bot/trader/Trader.cpp)

target_include_directories(stock_emu_lib PRIVATE ../)

target_compile_options(stock_emu_lib PRIVATE
    -Wall -O0 -g 
)

