cmake_minimum_required(VERSION 3.10)
project(452-phase C)

set(CMAKE_C_STANDARD 99)

include_directories(../usloss/src)
include_directories(../usloss/libuser)
include_directories(../usloss/libdisk)
include_directories(~/include)
include_directories(~/lib)


include(./phase1/CMakeLists.txt)
include(./phase2/CMakeLists.txt)
include(./phase3/CMakeLists.txt)