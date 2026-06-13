cmake_minimum_required(VERSION 3.16)

project(encryption_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

add_executable(encryption-engine
    src/engine.cpp
    src/main.cpp
)

target_include_directories(encryption-engine PRIVATE
    include
    ${SODIUM_INCLUDE_DIRS}
)

target_link_libraries(encryption-engine PRIVATE
    ${SODIUM_LIBRARIES}
)

target_compile_options(encryption-engine PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
