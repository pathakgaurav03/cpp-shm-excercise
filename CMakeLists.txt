cmake_minimum_required(VERSION 3.13)
project(TakeItToProduction)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(external)

add_compile_options(
        -Wcast-align
        -Wpedantic
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wnon-virtual-dtor
        -Wmissing-include-dirs
        -Wdisabled-optimization
        -Winit-self
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wparentheses
        -Wredundant-decls
        -Wshadow
        -Wsign-promo
        -Wstrict-aliasing
        -Wall
)

add_library(image_ipc SHARED
example/image_ipc.cpp
)

target_link_libraries(image_ipc PUBLIC boost rt pthread)

enable_testing()
add_subdirectory(test)
