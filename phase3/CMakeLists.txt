cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)


include_directories(phase3)

# for 3a

file(GLOB_RECURSE p3a_test_SOURCES "phase3/phase3a/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p3a_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase3a_test ${p3a_test_SOURCES})

add_library(phase3a phase3/phase3a/phase3a.c)


# for 3b

file(GLOB_RECURSE p3b_test_SOURCES "phase3/phase3b/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p3b_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase3b_test ${p3b_test_SOURCES})

add_library(phase3b phase3/phase3b/phase3b.c)


# for 3c

file(GLOB_RECURSE p3c_test_SOURCES "phase3/phase3c/tests/*.c")


set (INCLUDE_DIRS "")
foreach (_File ${p3c_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase3c_test ${p3c_test_SOURCES})

add_library(phase3c phase3/phase3c/phase3c.c)


# for 3d

file(GLOB_RECURSE p3d_test_SOURCES "phase3/phase3d/tests/*.c")


set(INCLUDE_DIRS "")
foreach (_File ${p3d_test_SOURCES})
    get_filename_component(_dir ${_File} PATH)
    list(APPEND INCLUDE_DIRS ${_dir})
endforeach ()
list(REMOVE_DUPLICATES INCLUDE_DIRS)


add_library(phase3d_test ${p3d_test_SOURCES})

add_library(phase3d phase3/phase3d/phase3d.c)