#include "two_phase_merge_sort.h"
#include "merge_utils.h"

#include <algorithm>
#include <fstream>
#include <utility>
#include <vector>

namespace {

template <typename T>
std::size_t distribution_phase(const std::filesystem::path& in,
                               const std::filesystem::path& g,
                               const std::filesystem::path& t,
                               std::size_t run_size,
                               bool sort_runs) {
    merge_detail::ElementReader<T> source(in);
    std::ofstream out_g(g, std::ios::binary | std::ios::trunc);
    std::ofstream out_t(t, std::ios::binary | std::ios::trunc);

    std::size_t run_count = 0;
    bool to_g = true;
    while (true) {
        std::vector<T> run = merge_detail::read_fixed_run<T>(source, run_size);
        if (run.empty()) {
            break;
        }
        if (sort_runs) {
            std::sort(run.begin(), run.end());
        }
        merge_detail::write_vector(to_g ? out_g : out_t, run);
        to_g = !to_g;
        ++run_count;
    }
    return run_count;
}

template <typename T>
void merge_phase(const std::filesystem::path& g,
                 const std::filesystem::path& t,
                 const std::filesystem::path& f,
                 std::size_t run_size) {
    merge_detail::ElementReader<T> in_g(g);
    merge_detail::ElementReader<T> in_t(t);
    std::ofstream out(f, std::ios::binary | std::ios::trunc);

    while (true) {
        std::vector<T> left = merge_detail::read_fixed_run<T>(in_g, run_size);
        std::vector<T> right = merge_detail::read_fixed_run<T>(in_t, run_size);
        if (left.empty() && right.empty()) {
            break;
        }
        merge_detail::write_merged(out, std::move(left), std::move(right));
    }
}

}

template <typename T>
void two_phase_merge_sort(const std::filesystem::path& input_file,
                          const std::filesystem::path& temp_dir,
                          std::size_t run_size) {
    if (!std::filesystem::exists(input_file)) {
        return;
    }
    std::filesystem::create_directories(temp_dir);
    if (run_size == 0) {
        run_size = 1;
    }

    const std::filesystem::path g = temp_dir / "two_phase_g.bin";
    const std::filesystem::path t = temp_dir / "two_phase_t.bin";

    std::size_t cur_size = run_size;
    std::size_t run_count = distribution_phase<T>(input_file, g, t, cur_size, true);

    if (run_count == 0) {
        merge_detail::remove_if_exists(g);
        merge_detail::remove_if_exists(t);
        return;
    }

    bool result_in_input = false;
    while (true) {
        merge_phase<T>(g, t, input_file, cur_size);
        result_in_input = true;

        run_count = (run_count + 1) / 2;
        if (run_count == 1) {
            break;
        }

        cur_size *= 2;
        run_count = distribution_phase<T>(input_file, g, t, cur_size, false);
        result_in_input = false;
    }

    if (!result_in_input) {
        merge_detail::replace_file(g, input_file);
    }

    merge_detail::remove_if_exists(g);
    merge_detail::remove_if_exists(t);
}

template void two_phase_merge_sort<int>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
template void two_phase_merge_sort<double>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
template void two_phase_merge_sort<float>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
