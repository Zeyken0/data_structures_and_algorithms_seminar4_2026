#include "natural_merge_sort.h"
#include "merge_utils.h"

#include <vector>

namespace {

template <typename T>
void natural_pass(const std::filesystem::path& in_a,
                  const std::filesystem::path& in_b,
                  const std::filesystem::path& out_a,
                  const std::filesystem::path& out_b) {
    merge_detail::ElementStream<T> a(in_a);
    merge_detail::ElementStream<T> b(in_b);
    std::ofstream out_0(out_a, std::ios::binary | std::ios::trunc);
    std::ofstream out_1(out_b, std::ios::binary | std::ios::trunc);

    std::size_t emitted = 0;
    auto emit = [&](std::vector<T> run) {
        if (emitted % 2 == 0) {
            merge_detail::write_vector(out_0, run);
        } else {
            merge_detail::write_vector(out_1, run);
        }
        ++emitted;
    };

    while (a.valid && b.valid) {
        emit(merge_detail::merge_vectors(merge_detail::read_run(a), merge_detail::read_run(b)));
    }
    while (a.valid) emit(merge_detail::read_run(a));
    while (b.valid) emit(merge_detail::read_run(b));
}

}

template <typename T>
void natural_merge_sort(const std::filesystem::path& input_file,
                        const std::filesystem::path& temp_dir) {
    if (!std::filesystem::exists(input_file)) return;
    std::filesystem::create_directories(temp_dir);

    const std::filesystem::path file_0 = temp_dir / "natural_0.bin";
    const std::filesystem::path file_1 = temp_dir / "natural_1.bin";
    const std::filesystem::path file_2 = temp_dir / "natural_2.bin";
    const std::filesystem::path file_3 = temp_dir / "natural_3.bin";

    {
        merge_detail::ElementStream<T> src(input_file);
        if (!src.valid) return;

        std::vector<T> first_run = merge_detail::read_run(src);
        if (!src.valid) return;

        std::ofstream out_0(file_0, std::ios::binary | std::ios::trunc);
        std::ofstream out_1(file_1, std::ios::binary | std::ios::trunc);

        std::size_t runs = 1;
        T last = first_run.back();
        while (src.valid) {
            std::vector<T> run = merge_detail::read_run(src);
            last = run.back();
            if (runs % 2 == 1) {
                merge_detail::write_vector(out_1, run);
            } else {
                merge_detail::write_vector(out_0, run);
            }
            ++runs;
        }

        if (first_run.front() >= last) {
            if ((runs - 1) % 2 == 1) {
                merge_detail::write_vector(out_1, first_run);
            } else {
                merge_detail::write_vector(out_0, first_run);
            }
        } else {
            merge_detail::write_vector(out_0, first_run);
        }
    }

    std::size_t total = merge_detail::count_runs<T>(file_0) + merge_detail::count_runs<T>(file_1);
    bool results_in_first_pair = true;

    while (total > 1) {
        if (results_in_first_pair) {
            natural_pass<T>(file_0, file_1, file_2, file_3);
            total = merge_detail::count_runs<T>(file_2) + merge_detail::count_runs<T>(file_3);
        } else {
            natural_pass<T>(file_2, file_3, file_0, file_1);
            total = merge_detail::count_runs<T>(file_0) + merge_detail::count_runs<T>(file_1);
        }
        results_in_first_pair = !results_in_first_pair;
    }

    const std::filesystem::path& result_a = results_in_first_pair ? file_0 : file_2;
    const std::filesystem::path& result_b = results_in_first_pair ? file_1 : file_3;
    std::filesystem::path result = merge_detail::count_runs<T>(result_a) > 0 ? result_a : result_b;

    for (const std::filesystem::path& path : {file_0, file_1, file_2, file_3}) {
        if (path != result) merge_detail::remove_if_exists(path);
    }
    merge_detail::replace_file(result, input_file);
}

template void natural_merge_sort<int>(const std::filesystem::path&, const std::filesystem::path&);
template void natural_merge_sort<double>(const std::filesystem::path&, const std::filesystem::path&);
template void natural_merge_sort<float>(const std::filesystem::path&, const std::filesystem::path&);
