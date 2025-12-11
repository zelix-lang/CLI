include(FetchContent)
cmake_minimum_required(VERSION 3.14)
project(ZelixCLI C CXX)

FetchContent_Declare(
        Celery
        GIT_REPOSITORY https://github.com/rodrigoo-r/Celery
        GIT_TAG        master
)

FetchContent_Declare(
        unordered_dense
        GIT_REPOSITORY https://github.com/martinus/unordered_dense
        GIT_TAG        main
)

FetchContent_MakeAvailable(Celery)
FetchContent_MakeAvailable(unordered_dense)
set(CMAKE_CXX_STANDARD 20)

add_library(ZelixCLI INTERFACE)
add_library(zelix::cli ALIAS ZelixCLI)

target_include_directories(
        ZelixCLI
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(ZelixCLI INTERFACE Celery::Celery)
target_link_libraries(ZelixCLI INTERFACE unordered_dense)
target_include_directories(ZelixCLI INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/extras)