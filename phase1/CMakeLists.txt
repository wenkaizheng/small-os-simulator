cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)


include_directories(phase1)


# for 1a


file(GLOB_RECURSE p1a_test_SOURCES "phase1/phase1a/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p1a_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase1a_test ${p1a_test_SOURCES})

add_library(phase1a phase1/phase1a/phase1a.c)


# for 1b


file(GLOB_RECURSE p1b_test_SOURCES "phase1/phase1b/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p1b_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase1b_test ${p1b_test_SOURCES})

add_library(phase1b phase1/phase1b/phase1b.c)


# for 1c



file(GLOB_RECURSE p1c_test_SOURCES "phase1/phase1c/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p1c_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase1c_test ${p1c_test_SOURCES})

add_library(phase1c phase1/phase1c/phase1c.c)


# for 1D



file(GLOB_RECURSE p1d_test_SOURCES "phase1/phase1d/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p1d_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase1d_test ${p1d_test_SOURCES})

add_library(phase1d phase1/phase1d/phase1d.c)
