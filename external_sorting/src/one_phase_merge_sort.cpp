#include "one_phase_merge_sort.h"
#include "merge_utils.h"

#include <utility>

template <typename T>
void one_phase_merge_sort(const std::filesystem::path& input_file,
                          const std::filesystem::path& temp_dir,
                          std::size_t run_size) {
    if (!std::filesystem::exists(input_file)) return;
    std::filesystem::create_directories(temp_dir);
    if (run_size == 0) run_size = 1;

    std::filesystem::path accumulator;
    std::filesystem::path other;
    {
        std::ifstream in(input_file, std::ios::binary);
        accumulator = merge_detail::make_path(temp_dir, "one_phase_acc_", 0);
        other = merge_detail::make_path(temp_dir, "one_phase_acc_", 1);
        bool empty = true;

        while (true) {
            std::vector<T> block = merge_detail::read_block<T>(in, run_size);
            if (block.empty()) break;
            merge_detail::sort_block(block);

            if (empty) {
                std::ofstream out(accumulator, std::ios::binary | std::ios::trunc);
                merge_detail::write_vector(out, block);
                empty = false;
            } else {
                merge_detail::merge_file_with_block<T>(accumulator, block, other);
                merge_detail::remove_if_exists(accumulator);
                std::swap(accumulator, other);
            }
        }
        if (empty) return;
    }

    merge_detail::replace_file(accumulator, input_file);
    merge_detail::remove_if_exists(other);
}

template void one_phase_merge_sort<int>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
template void one_phase_merge_sort<double>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
template void one_phase_merge_sort<float>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
