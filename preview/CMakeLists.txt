project(preview)

project(stock_emu_test CXX)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-stdlib=libc++)
add_link_options(-stdlib=libc++)
add_executable(stock_emu_preview
    ./main.cpp
)

target_include_directories(stock_emu_test PRIVATE ../)
target_link_libraries(stock_emu_test PRIVATE
    stock_emu_lib
)
target_compile_options(stock_emu_test PRIVATE
    -Wall -O0 -g -stdlib=libc++
)
target_link_options(stock_emu_test PRIVATE
    -stdlib=libc++
)

