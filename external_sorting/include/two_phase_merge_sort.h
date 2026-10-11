#pragma once

#include <cstddef>
#include <filesystem>

template <typename T>
void two_phase_merge_sort(const std::filesystem::path& input_file,
                          const std::filesystem::path& temp_dir,
                          std::size_t run_size = 1);
