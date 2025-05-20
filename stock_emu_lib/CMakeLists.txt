project(stock_emu_lib CXX)
set(CMAKE_CXX_STANDARD 20)
add_library(stock_emu_lib STATIC 
    trade/Trade.cpp
    trade/TradeHistory.cpp
    trade/PriceLimit.cpp
    bot/trader/Trader.cpp)

target_include_directories(stock_emu_lib PRIVATE ../)


if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
target_compile_options(stock_emu_lib PRIVATE
    -Wall -Weverything
    -Wno-c++98-compat -Wno-padded -Wno-exit-time-destructors -Wno-global-constructors
) 
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
target_compile_options(stock_emu_lib PRIVATE
    -Wall
) 
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
target_compile_options(stock_emu_lib PRIVATE
    /Wall
) 
endif()
