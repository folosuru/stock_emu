project(stock_emu_test CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(stock_emu_test 
    ./main.cpp
    #./trade/trade.cpp
    ./Fuzzy.cpp
)

target_include_directories(stock_emu_test PRIVATE ../)

target_link_libraries(stock_emu_test PRIVATE
    # stock_emu_lib
    GTest::gtest_main
)
target_compile_options(stock_emu_test PRIVATE
    -Wall -O0 -g 
)
include(GoogleTest)
gtest_discover_tests(stock_emu_test)

