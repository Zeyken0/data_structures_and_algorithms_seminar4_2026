#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "natural_merge_sort.h"
#include "test_utils.h"
#include <string>
#include <vector>

namespace {

template <typename T>
void check_sorted(const std::string& case_name,
                  const std::vector<T>& input,
                  const std::vector<T>& expected) {
    TempDir dir(case_name);
    const std::filesystem::path input_file = dir / "input.bin";
    const std::filesystem::path temp_dir = dir / "temp";

    write_vector_to_file(input_file, input);
    natural_merge_sort<T>(input_file, temp_dir);

    REQUIRE(read_vector_from_file<T>(input_file) == expected);
    // Вспомогательные файлы g и t должны быть удалены.
    REQUIRE(std::filesystem::is_empty(temp_dir));
}

}

TEST_CASE("NaturalMergeSort: пустой файл", "[natural_merge_sort]") {
    check_sorted<int>("natural_empty", {}, {});
}

TEST_CASE("NaturalMergeSort: один элемент", "[natural_merge_sort]") {
    check_sorted<int>("natural_single", {42}, {42});
}

TEST_CASE("NaturalMergeSort: уже отсортированный", "[natural_merge_sort]") {
    // Файл состоит из одной естественной серии — фаза слияния не нужна.
    check_sorted<int>("natural_sorted", {1, 2, 3, 4, 5}, {1, 2, 3, 4, 5});
}

TEST_CASE("NaturalMergeSort: обратный порядок", "[natural_merge_sort]") {
    check_sorted<int>("natural_reversed", {5, 4, 3, 2, 1}, {1, 2, 3, 4, 5});
}

TEST_CASE("NaturalMergeSort: произвольный порядок", "[natural_merge_sort]") {
    check_sorted<int>("natural_arbitrary",
                      {3, 1, 4, 1, 5, 9, 2, 6, 5, 3},
                      {1, 1, 2, 3, 3, 4, 5, 5, 6, 9});
}

TEST_CASE("NaturalMergeSort: много дубликатов", "[natural_merge_sort]") {
    // Равные подряд идущие элементы образуют одну серию.
    check_sorted<int>("natural_duplicates", {5, 1, 5, 1, 5, 1}, {1, 1, 1, 5, 5, 5});
}

TEST_CASE("NaturalMergeSort: отрицательные числа", "[natural_merge_sort]") {
    check_sorted<int>("natural_negative", {-1, -5, -3, -2, -4}, {-5, -4, -3, -2, -1});
}

TEST_CASE("NaturalMergeSort: две чередующиеся серии", "[natural_merge_sort]") {
    check_sorted<int>("natural_two_runs",
                      {1, 2, 3, 4, 10, 11, 12, 5, 6, 7, 20, 21},
                      {1, 2, 3, 4, 5, 6, 7, 10, 11, 12, 20, 21});
}

TEST_CASE("NaturalMergeSort: склейка первой и последней серии", "[natural_merge_sort]") {
    check_sorted<int>("natural_glue_runs", {4, 5, 6, 1, 2, 3}, {1, 2, 3, 4, 5, 6});
}

TEST_CASE("NaturalMergeSort: серия больше прочих", "[natural_merge_sort]") {
    check_sorted<int>("natural_big_run",
                      {9, 8, 7, 1, 2, 3, 4, 5, 6},
                      {1, 2, 3, 4, 5, 6, 7, 8, 9});
}

TEST_CASE("NaturalMergeSort: пример из лекции", "[natural_merge_sort]") {
    // Исходный файл f: 3 14 15 9 2 5 17 -4 0
    // 1-й проход. Распределение: g: 3 14 15 | 2 5 17, t: 9 | -4 0
    //               Слияние:     f: 3 9 14 15 | -4 0 2 5 17
    // 2-й проход: g: 3 9 14 15, t: -4 0 2 5 17 -> одна серия.
    check_sorted<int>("natural_lecture_example",
                      {3, 14, 15, 9, 2, 5, 17, -4, 0},
                      {-4, 0, 2, 3, 5, 9, 14, 15, 17});
}

TEST_CASE("NaturalMergeSort: серия длиной 1 на каждой позиции", "[natural_merge_sort]") {
    // Файл распадается на максимально возможное число серий: 20 1 | 40 2 | ...
    const std::vector<int> input = make_alternating_values(20);
    check_sorted<int>("natural_alternating", input, sorted_copy(input));
}

TEST_CASE("NaturalMergeSort: серия занимает почти весь файл", "[natural_merge_sort]") {
    std::vector<int> input;
    for (int i = 1; i <= 50; ++i) {
        input.push_back(i);
    }
    input.push_back(0);
    check_sorted<int>("natural_almost_one_run", input, sorted_copy(input));
}

TEST_CASE("NaturalMergeSort: случайный массив", "[natural_merge_sort]") {
    SECTION("10000 элементов") {
        const std::vector<int> input = make_random_ints(10000, 42);
        check_sorted<int>("natural_random", input, sorted_copy(input));
    }
    SECTION("9999 элементов") {
        const std::vector<int> input = make_random_ints(9999, 777);
        check_sorted<int>("natural_random_9999", input, sorted_copy(input));
    }
}

TEST_CASE("NaturalMergeSort: два элемента, оба порядка", "[natural_merge_sort]") {
    SECTION("по возрастанию") {
        check_sorted<int>("natural_two_asc", {1, 2}, {1, 2});
    }
    SECTION("по убыванию") {
        check_sorted<int>("natural_two_desc", {2, 1}, {1, 2});
    }
}

TEST_CASE("NaturalMergeSort: double", "[natural_merge_sort][double]") {
    SECTION("положительные дробные") {
        check_sorted<double>("natural_double_pos",
                             {3.5, 1.25, 2.75, 0.5, 4.0},
                             {0.5, 1.25, 2.75, 3.5, 4.0});
    }
    SECTION("отрицательные и положительные") {
        check_sorted<double>("natural_double_mix",
                             {-1.5, 2.25, -3.75, 0.0, 1.5},
                             {-3.75, -1.5, 0.0, 1.5, 2.25});
    }
    SECTION("случайные дробные") {
        const std::vector<double> input = make_random_doubles(500, 13);
        check_sorted<double>("natural_double_random", input, sorted_copy(input));
    }
    SECTION("пустой файл") {
        check_sorted<double>("natural_double_empty", {}, {});
    }
}

TEST_CASE("NaturalMergeSort: float", "[natural_merge_sort][float]") {
    check_sorted<float>("natural_float",
                        {2.5F, -1.0F, 0.0F, 7.25F, -1.0F},
                        {-1.0F, -1.0F, 0.0F, 2.5F, 7.25F});
}
