#pragma once

#include <filesystem>

template <typename T>
void natural_merge_sort(const std::filesystem::path& input_file,
                        const std::filesystem::path& temp_dir);
