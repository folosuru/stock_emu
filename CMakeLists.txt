
project(stock_emu CXX)
set(CMAKE_CXX_STANDARD 20)
add_executable(emu src/main.cpp src/Trade.cpp src/bot/trader/Trader.cpp)
target_include_directories(emu PRIVATE src/)
target_compile_options(emu PRIVATE
    -Wall -O3
)
