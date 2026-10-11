#include "two_phase_merge_sort.h"
#include "merge_utils.h"

#include <algorithm>
#include <fstream>
#include <utility>
#include <vector>

namespace {

// Фаза разделения: файл in распределяется по файлам g и t сериями длиной
// run_size попеременно (первая серия — в g, вторая — в t, третья — в g и т.д.).
// Возвращает количество серий (в g попадает на одну серию больше, чем в t).
//
// sort_runs == true — каждая считанная серия предварительно сортируется в ОП
// (так обрабатывается исходный файл). Иначе series в файле in уже упорядочены
// и лежат подряд, поэтому достаточно их скопировать.
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

// Фаза слияния: серии файлов g и t сливается попарно в файл f.
// Если серий в одном файле оказалось больше, "лишние" просто дозаписываются
// в конец — их элементы не меньше всех уже записанных.
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

// Алгоритм 2-х фазной сортировки прямым слиянием.
//
// Используется три файла: f (исходный) и два вспомогательных — g и t.
//
// Фаза разделения: исходный файл f читается сериями, каждая серия сортируется
// в ОП и записывается в файлы g и t попеременно.
// Фаза слияния: серии файлов g и t сливается попарно обратно в f — длина
// серий удваивается.
// Проходы повторяются, пока в файле f не останется одна серия.
//
// run_size = 1 — классическая схема из лекции (начальные серии длиной 1).
// run_size > 1 — ускорение прямого слияния: блоки из run_size записей
// сортируются в ОП и во внешнюю память попадают уже в виде серий длиной k.
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

    // Начальное разделение исходного файла на серии с их сортировкой в ОП.
    std::size_t cur_size = run_size;
    std::size_t run_count = distribution_phase<T>(input_file, g, t, cur_size, true);

    if (run_count == 0) {
        // Пустой исходный файл — сортировать нечего.
        merge_detail::remove_if_exists(g);
        merge_detail::remove_if_exists(t);
        return;
    }

    // Проходы сортировки. После k-го слияния серии файла f имеют длину
    // 2^k * run_size (кроме последней), поэтому длина читаемой серии
    // на каждом проходе удваивается.
    bool result_in_input = false;
    while (true) {
        merge_phase<T>(g, t, input_file, cur_size);
        result_in_input = true;

        run_count = (run_count + 1) / 2;
        if (run_count == 1) {
            // В файле f осталась одна серия — сортировка закончена.
            break;
        }

        cur_size *= 2;
        run_count = distribution_phase<T>(input_file, g, t, cur_size, false);
        result_in_input = false;
    }

    if (!result_in_input) {
        // Сортировка завершилась ещё на фазе разделения: файл f содержит одну
        // серию, которая лежит в файле g — возвращаем её в исходный файл.
        merge_detail::replace_file(g, input_file);
    }

    merge_detail::remove_if_exists(g);
    merge_detail::remove_if_exists(t);
}

template void two_phase_merge_sort<int>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
template void two_phase_merge_sort<double>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
template void two_phase_merge_sort<float>(const std::filesystem::path&, const std::filesystem::path&, std::size_t);
