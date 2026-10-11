#include "natural_merge_sort.h"
#include "merge_utils.h"

#include <fstream>
#include <utility>
#include <vector>

namespace {

// Фаза распределения: естественные серии файла f попеременно распределяются
// по файлам g и t (первая серия — в g, вторая — в t и т.д.). Длина каждой
// серии определяется данными, а не задаётся заранее.
// Возвращает количество серий; если серия одна, файл f уже отсортирован
// и ни в g, ни в t ничего не записывается.
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
            // Весь файл оказался одной серией — распределять нечего.
            break;
        }
        merge_detail::write_vector(to_g ? out_g : out_t, run);
        to_g = !to_g;
    }
    return run_count;
}

// Фаза слияния: естественные серии файлов g и t сливается попарно в файл f.
// Если серий в одном файле оказалось больше, "лишние" дозаписываются в конец —
// все элементы следующей серии не меньше всех элементов предыдущей.
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

// Алгоритм сортировки естественным слиянием.
//
// Используется три файла: f (исходный) и два вспомогательных — g и t.
// В отличие от прямого слияния, длина сливаемых серий не фиксируется:
//series берутся ровно такими, какими их задают данные.
//
// Фаза распределения:series файла f попеременно распределяются по g и t.
// Фаза слияния:series файлов g и t сливается попарно обратно в f.
// Проходы повторяются, пока в файле f не останется одна серия.
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
            // В файле f одна серия (или он пуст) — сортировка закончена.
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
