add_library(external_sorting STATIC
    src/natural_merge_sort.cpp
    src/one_phase_merge_sort.cpp
    src/two_phase_merge_sort.cpp
)

target_include_directories(external_sorting PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(external_sorting PRIVATE
    -Wall -Wextra -Wpedantic
)
