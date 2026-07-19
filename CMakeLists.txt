cmake_minimum_required(VERSION 3.16)
project(drone_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(drone_sim
    src/main.cpp
)

target_include_directories(drone_sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})# Add Catch2 testing framework
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.15.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(drone_tests
    test/test.cpp
)
target_link_libraries(drone_tests PRIVATE Catch2::Catch2WithMain)

enable_testing()
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(drone_tests)