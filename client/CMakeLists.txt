cmake_minimum_required(VERSION 3.5)

project(ftp-client)

set(CMAKE_CXX_STANDARD 20)

set(
    ${PROJECT_NAME}_COMPILE_FLAGS
    "-Wall" "-Wextra" "-Wpedantic" "-Werror" "-pedantic-errors"
)

file(
    GLOB_RECURSE
    ${PROJECT_NAME}_SOURCES
    "src/*.cpp"
)

add_executable(
    ${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        inc/
)