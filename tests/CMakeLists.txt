cmake_minimum_required(VERSION 3.22)
project(cmocka-examples C)

set_source_files_properties(PROPERTIES
                                COMPILE_DEFINITIONS
                                    UNIT_TESTING=1)


if (NOT WIN32)
    add_subdirectory(unit)
endif()
