set(CATCH2_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/catch2)

add_executable(sorting_tests
    test_natural_merge_sort.cpp
    test_one_phase_merge_sort.cpp
    test_two_phase_merge_sort.cpp
)

target_include_directories(sorting_tests PRIVATE ${CATCH2_INCLUDE_DIR})

target_link_libraries(sorting_tests PRIVATE external_sorting)

target_compile_options(sorting_tests PRIVATE
    -Wall -Wextra -Wpedantic
)

add_test(NAME sorting_tests COMMAND sorting_tests)
