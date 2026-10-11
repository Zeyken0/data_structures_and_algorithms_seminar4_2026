#include "natural_merge_sort.h"
#include "merge_utils.h"

#include <fstream>
#include <utility>
#include <vector>

namespace {

template <typename T>
std::size_t distribution_phase(const std::filesystem::path& f,
                               const std::filesystem::path& g,
                               const std::filesystem::path& t) {
    merge_detail::ElementReader<T> source(f);
    std::ofstream out_g(g, std::ios::binary | std::ios::trunc);
    std::ofstream out_t(t, std::ios::binary | std::ios::trunc);

    std::size_t run_count = 0;
    bool to_g = true;
    while (true) {
        std::vector<T> run = merge_detail::read_natural_run<T>(source);
        if (run.empty()) {
            break;
        }
        ++run_count;
        if (!source.valid() && run_count == 1) {
            break;
        }
        merge_detail::write_vector(to_g ? out_g : out_t, run);
        to_g = !to_g;
    }
    return run_count;
}

template <typename T>
void merge_phase(const std::filesystem::path& g,
                 const std::filesystem::path& t,
                 const std::filesystem::path& f) {
    merge_detail::ElementReader<T> in_g(g);
    merge_detail::ElementReader<T> in_t(t);
    std::ofstream out(f, std::ios::binary | std::ios::trunc);

    while (true) {
        std::vector<T> left = merge_detail::read_natural_run<T>(in_g);
        std::vector<T> right = merge_detail::read_natural_run<T>(in_t);
        if (left.empty() && right.empty()) {
            break;
        }
        merge_detail::write_merged(out, std::move(left), std::move(right));
    }
}

}

template <typename T>
void natural_merge_sort(const std::filesystem::path& input_file,
                        const std::filesystem::path& temp_dir) {
    if (!std::filesystem::exists(input_file)) {
        return;
    }
    std::filesystem::create_directories(temp_dir);

    const std::filesystem::path g = temp_dir / "natural_g.bin";
    const std::filesystem::path t = temp_dir / "natural_t.bin";

    while (true) {
        const std::size_t run_count = distribution_phase<T>(input_file, g, t);
        if (run_count <= 1) {
            break;
        }
        merge_phase<T>(g, t, input_file);
    }

    merge_detail::remove_if_exists(g);
    merge_detail::remove_if_exists(t);
}

template void natural_merge_sort<int>(const std::filesystem::path&, const std::filesystem::path&);
template void natural_merge_sort<double>(const std::filesystem::path&, const std::filesystem::path&);
template void natural_merge_sort<float>(const std::filesystem::path&, const std::filesystem::path&);
