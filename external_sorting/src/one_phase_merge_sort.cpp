#include "one_phase_merge_sort.h"
#include "merge_utils.h"

#include <algorithm>
#include <fstream>
#include <utility>
#include <vector>

namespace {

template <typename T>
std::size_t split_into_files(const std::filesystem::path& input_file,
                             const std::filesystem::path& f1,
                             const std::filesystem::path& f2,
                             std::size_t run_size) {
    merge_detail::ElementReader<T> source(input_file);
    std::ofstream out_1(f1, std::ios::binary | std::ios::trunc);
    std::ofstream out_2(f2, std::ios::binary | std::ios::trunc);

    std::size_t run_count = 0;
    bool to_first = true;
    while (true) {
        std::vector<T> run = merge_detail::read_fixed_run<T>(source, run_size);
        if (run.empty()) {
            break;
        }
        std::sort(run.begin(), run.end());
        merge_detail::write_vector(to_first ? out_1 : out_2, run);
        to_first = !to_first;
        ++run_count;
    }
    return run_count;
}

}

template <typename T>
void one_phase_merge_sort(const std::filesystem::path& input_file,
                          const std::filesystem::path& temp_dir,
                          std::size_t run_size) {
    if (!std::filesystem::exists(input_file)) {
        return;
    }
    std::filesystem::create_directories(temp_dir);
    if (run_size == 0) {
        run_size = 1;
    }

    const std::filesystem::path f1 = temp_dir / "one_phase_f1.bin";
    const std::filesystem::path f2 = temp_dir / "one_phase_f2.bin";
    const std::filesystem::path g1 = temp_dir / "one_phase_g1.bin";
    const std::filesystem::path g2 = temp_dir / "one_phase_g2.bin";

    std::size_t cur_size = run_size;
    std::size_t run_count = split_into_files<T>(input_file, f1, f2, cur_size);

    if (run_count == 0) {
        for (const std::filesystem::path& path : {f1, f2, g1, g2}) {
            merge_detail::remove_if_exists(path);
        }
        return;
    }

    std::filesystem::path src_1 = f1;
    std::filesystem::path src_2 = f2;
    std::filesystem::path dst_1 = g1;
    std::filesystem::path dst_2 = g2;

    while (run_count > 1) {
        {
            merge_detail::ElementReader<T> in_1(src_1);
            merge_detail::ElementReader<T> in_2(src_2);
            std::ofstream out_1(dst_1, std::ios::binary | std::ios::trunc);
            std::ofstream out_2(dst_2, std::ios::binary | std::ios::trunc);

            std::size_t emitted = 0;
            while (true) {
                std::vector<T> left = merge_detail::read_fixed_run<T>(in_1, cur_size);
                std::vector<T> right = merge_detail::read_fixed_run<T>(in_2, cur_size);
                if (left.empty() && right.empty()) {
                    break;
                }
                merge_detail::write_merged(emitted % 2 == 0 ? out_1 : out_2,
                                           std::move(left), std::move(right));
                ++emitted;
            }
        }

        run_count = (run_count + 1) / 2;
        cur_size *= 2;

        std::swap(src_1, dst_1);
        std::swap(src_2, dst_2);
    }

    const std::filesystem::path result =
        merge_detail::file_is_non_empty(src_1) ? src_1 : src_2;
    merge_detail::replace_file(result, input_file);

    for (const std::filesystem::path& path : {f1, f2, g1, g2}) {
        if (path != result) {
            merge_detail::remove_if_exists(path);
        }
    }
}

template void one_phase_merge_sort<int>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
template void one_phase_merge_sort<double>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
template void one_phase_merge_sort<float>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
