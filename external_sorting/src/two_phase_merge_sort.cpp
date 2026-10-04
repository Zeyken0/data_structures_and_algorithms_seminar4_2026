#include "two_phase_merge_sort.h"
#include "merge_utils.h"

template <typename T>
void two_phase_merge_sort(const std::filesystem::path& input_file,
                          const std::filesystem::path& temp_dir,
                          std::size_t run_size) {
    if (!std::filesystem::exists(input_file)) return;
    std::filesystem::create_directories(temp_dir);
    if (run_size == 0) run_size = 1;

    std::vector<std::filesystem::path> runs;
    {
        std::ifstream in(input_file, std::ios::binary);
        while (true) {
            std::vector<T> block = merge_detail::read_block<T>(in, run_size);
            if (block.empty()) break;
            merge_detail::sort_block(block);

            std::filesystem::path run_path = merge_detail::make_path(temp_dir, "two_phase_run_", runs.size());
            std::ofstream out(run_path, std::ios::binary | std::ios::trunc);
            merge_detail::write_vector(out, block);
            runs.push_back(run_path);
        }
    }

    if (runs.empty()) return;
    if (runs.size() == 1) {
        merge_detail::replace_file(runs.front(), input_file);
        return;
    }

    std::size_t generation = 0;
    while (runs.size() > 1) {
        const std::string prefix = (generation % 2 == 0) ? "two_phase_a_" : "two_phase_b_";
        std::vector<std::filesystem::path> next;
        for (std::size_t i = 0; i + 1 < runs.size(); i += 2) {
            std::filesystem::path merged_path = merge_detail::make_path(temp_dir, prefix, next.size());
            merge_detail::merge_files<T>(runs[i], runs[i + 1], merged_path);
            next.push_back(merged_path);
        }
        if (runs.size() % 2 == 1) {
            next.push_back(runs.back());
        }
        for (const std::filesystem::path& run : runs) {
            if (std::find(next.begin(), next.end(), run) == next.end()) {
                merge_detail::remove_if_exists(run);
            }
        }
        runs = std::move(next);
        ++generation;
    }

    merge_detail::replace_file(runs.front(), input_file);
}

template void two_phase_merge_sort<int>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
template void two_phase_merge_sort<double>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
template void two_phase_merge_sort<float>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
