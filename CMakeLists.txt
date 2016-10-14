cmake_minimum_required (VERSION 3.1)
project (mancala)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build the unit tests" ON)
option(BUILD_BOT "Build the game playing bot" ON)


# Build the core mancala files as a library to make it easier to test

file(GLOB_RECURSE LIB_FILES "src/mancala/*.cpp" "src/mancala/*.c")

add_library(mancala STATIC ${LIB_FILES})
target_include_directories(mancala
    PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(mancala
    PRIVATE -Wall
    PRIVATE -Wextra
    PRIVATE -pedantic)


# Use googletest as our testing framework. If this part of configuring fails
# make sure you've run 'git submodule init && git submodule update'

if(BUILD_TESTS)
    enable_testing()
    set(BUILD_GTEST true CACHE BOOL "")
    set(BUILD_GMOCK false CACHE BOOL "")
    add_subdirectory(${CMAKE_SOURCE_DIR}/deps/googletest)

    add_executable(alltests "test/all_tests.cpp")
    target_link_libraries(alltests mancala gtest gtest_main)
    target_include_directories(
        alltests
        PRIVATE ${CMAKE_SOURCE_DIR}/src
        PRIVATE ${CMAKE_SOURCE_DIR}/test
        PRIVATE ${CMAKE_SOURCE_DIR}/deps/googletest/googletest/include)
    target_compile_options(
        alltests
        PRIVATE -Wall
        PRIVATE -Wextra
        PRIVATE -pedantic)

    add_test(NAME alltests COMMAND alltests)
endif()


# The actual game-playing executable

if(BUILD_BOT)
    file(GLOB_RECURSE BOT_FILES "src/bot/*.cpp" "src/bot/*.c")
    
    add_executable(bot ${BOT_FILES})
    target_link_libraries(bot mancala)
    target_include_directories(
        bot
        PRIVATE ${CMAKE_SOURCE_DIR}/src)
endif()
