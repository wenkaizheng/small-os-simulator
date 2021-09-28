cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)


include_directories(phase2)

# for 2a


file(GLOB_RECURSE p2a_test_SOURCES "phase2/phase2a/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p2a_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase2a_test ${p2a_test_SOURCES})

add_library(phase2a phase2/phase2a/phase2a.c)

# for 2b

file(GLOB_RECURSE p2b_test_SOURCES "phase2/phase2b/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p2b_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase2b_test ${p2b_test_SOURCES})

add_library(phase2b
        phase2/phase2b/phase2b.c
        phase2/tester.h)


# for 2c

file(GLOB_RECURSE p2c_test_SOURCES "phase2/phase2c/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p2c_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase2c_test ${p2c_test_SOURCES})

add_library(phase2c
        phase2/phase2c/phase2c.c
        phase2/tester.h)


# for 2d

file(GLOB_RECURSE p2d_test_SOURCES "phase2/phase2d/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p2d_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase2d_test ${p2d_test_SOURCES})

add_library(phase2d
        phase2/phase2d/phase2d.c
        phase2/tester.h)