cmake_minimum_required(VERSION 3.10)

project(libuci)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

file(GLOB SOURCES "*.c" "lib/mongoose.c")

add_executable(uci ${SOURCES})
