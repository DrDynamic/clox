cmake_minimum_required(VERSION 3.22)
project(clox-tests VERSION 1.0 LANGUAGES C)

include(FetchCMocka.cmake)


add_executable(clox-tests main.c)
target_compile_features(clox-tests PRIVATE c_std_99)
target_link_libraries(clox-tests PRIVATE cmocka-static)

set_source_files_properties(PROPERTIES
                                COMPILE_DEFINITIONS
                                    UNIT_TESTING=1)

enable_testing()
if (NOT WIN32)
    add_subdirectory(unit)
endif()
