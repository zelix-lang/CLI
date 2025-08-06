include(FetchContent)
cmake_minimum_required(VERSION 3.14)
project(FluentCLI)

FetchContent_Declare(
        fluent_stl
        GIT_REPOSITORY https://github.com/fluent-lang/STL.git
        GIT_TAG        master
)

FetchContent_Declare(
        unordered_dense
        GIT_REPOSITORY https://github.com/martinus/unordered_dense
        GIT_TAG        main
)

FetchContent_MakeAvailable(fluent_stl)
FetchContent_MakeAvailable(unordered_dense)
set(CMAKE_CXX_STANDARD 20)

add_library(FluentCLI INTERFACE)
add_library(fluent::cli ALIAS FluentCLI)

target_include_directories(
        FluentCLI
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(FluentCLI INTERFACE fluent::stl)
target_link_libraries(FluentCLI INTERFACE unordered_dense)
target_include_directories(FluentCLI INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/extras)