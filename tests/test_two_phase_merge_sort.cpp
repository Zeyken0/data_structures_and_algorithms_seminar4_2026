#include <catch.hpp>
#include "two_phase_merge_sort.h"
#include "test_utils.h"
#include <string>
#include <vector>

namespace {

template <typename T>
void check_sorted(const std::string& case_name,
                  const std::vector<T>& input,
                  const std::vector<T>& expected,
                  std::size_t run_size) {
    TempDir dir(case_name);
    const std::filesystem::path input_file = dir / "input.bin";
    const std::filesystem::path temp_dir = dir / "temp";

    write_vector_to_file(input_file, input);
    two_phase_merge_sort<T>(input_file, temp_dir, run_size);

    REQUIRE(read_vector_from_file<T>(input_file) == expected);
    // Вспомогательные файлы f, g и t должны быть удалены.
    REQUIRE(std::filesystem::is_empty(temp_dir));
}

}

TEST_CASE("TwoPhaseMergeSort: пустой файл", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_empty", {}, {}, 1);
}

TEST_CASE("TwoPhaseMergeSort: один элемент", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_single", {42}, {42}, 1);
}

TEST_CASE("TwoPhaseMergeSort: уже отсортированный", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_sorted", {1, 2, 3, 4, 5}, {1, 2, 3, 4, 5}, 2);
}

TEST_CASE("TwoPhaseMergeSort: обратный порядок", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_reversed", {5, 4, 3, 2, 1}, {1, 2, 3, 4, 5}, 2);
}

TEST_CASE("TwoPhaseMergeSort: произвольный порядок", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_arbitrary",
                      {3, 1, 4, 1, 5, 9, 2, 6, 5, 3},
                      {1, 1, 2, 3, 3, 4, 5, 5, 6, 9},
                      3);
}

TEST_CASE("TwoPhaseMergeSort: много дубликатов", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_duplicates", {5, 1, 5, 1, 5, 1}, {1, 1, 1, 5, 5, 5}, 2);
}

TEST_CASE("TwoPhaseMergeSort: отрицательные числа", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_negative", {-1, -5, -3, -2, -4}, {-5, -4, -3, -2, -1}, 2);
}

TEST_CASE("TwoPhaseMergeSort: размер серии 1", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_unit_runs",
                      {9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
                      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                      1);
}

TEST_CASE("TwoPhaseMergeSort: пример из лекции", "[two_phase_merge_sort]") {
    // Исходный файл f: 3 14 15 9 2 5 17 -4 0
    // 1-й проход. Разделение: g: 3 15 2 17 0, t: 14 9 5 -4.
    //             Слияние:    f: 3 14 | 9 15 | 2 5 | -4 17 | 0
    // Далее длина серий удваивается до единственной серии.
    check_sorted<int>("two_phase_lecture_example",
                      {3, 14, 15, 9, 2, 5, 17, -4, 0},
                      {-4, 0, 2, 3, 5, 9, 14, 15, 17},
                      1);
}

TEST_CASE("TwoPhaseMergeSort: размер серии не делится нацело (хвост)", "[two_phase_merge_sort]") {
    SECTION("хвост короче серии") {
        check_sorted<int>("two_phase_tail_short",
                          {9, 8, 7, 6, 5, 4, 3, 2, 1},
                          {1, 2, 3, 4, 5, 6, 7, 8, 9},
                          4);
    }
    SECTION("хвост из одного элемента") {
        check_sorted<int>("two_phase_tail_single",
                          {5, 4, 3, 2, 1, 0},
                          {0, 1, 2, 3, 4, 5},
                          5);
    }
}

TEST_CASE("TwoPhaseMergeSort: серий больше, чем степени двойки", "[two_phase_merge_sort]") {
    // 17 записей при размере серии 1: серий 17 -> 9 -> 5 -> 3 -> 2 -> 1,
    // т.е. на нескольких проходах попадаются «лишние» серии.
    std::vector<int> input;
    for (int i = 17; i >= 1; --i) {
        input.push_back(i);
    }
    check_sorted<int>("two_phase_odd_runs", input, sorted_copy(input), 1);
}

TEST_CASE("TwoPhaseMergeSort: случайный массив", "[two_phase_merge_sort]") {
    SECTION("10000 элементов, серии длиной 1") {
        const std::vector<int> input = make_random_ints(10000, 42);
        check_sorted<int>("two_phase_random_unit", input, sorted_copy(input), 1);
    }
    SECTION("9999 элементов, размер серии 64") {
        const std::vector<int> input = make_random_ints(9999, 777);
        check_sorted<int>("two_phase_random_blocks", input, sorted_copy(input), 64);
    }
}

TEST_CASE("TwoPhaseMergeSort: два элемента, оба порядка", "[two_phase_merge_sort]") {
    SECTION("по возрастанию") {
        check_sorted<int>("two_phase_two_asc", {1, 2}, {1, 2}, 1);
    }
    SECTION("по убыванию") {
        check_sorted<int>("two_phase_two_desc", {2, 1}, {1, 2}, 1);
    }
}

TEST_CASE("TwoPhaseMergeSort: double", "[two_phase_merge_sort][double]") {
    SECTION("положительные дробные") {
        check_sorted<double>("two_phase_double_pos",
                             {3.5, 1.25, 2.75, 0.5, 4.0},
                             {0.5, 1.25, 2.75, 3.5, 4.0},
                             2);
    }
    SECTION("отрицательные и положительные") {
        check_sorted<double>("two_phase_double_mix",
                             {-1.5, 2.25, -3.75, 0.0, 1.5},
                             {-3.75, -1.5, 0.0, 1.5, 2.25},
                             2);
    }
    SECTION("случайные дробные") {
        const std::vector<double> input = make_random_doubles(500, 13);
        check_sorted<double>("two_phase_double_random", input, sorted_copy(input), 1);
    }
    SECTION("пустой файл") {
        check_sorted<double>("two_phase_double_empty", {}, {}, 2);
    }
}

TEST_CASE("TwoPhaseMergeSort: float", "[two_phase_merge_sort][float]") {
    check_sorted<float>("two_phase_float",
                        {2.5F, -1.0F, 0.0F, 7.25F, -1.0F},
                        {-1.0F, -1.0F, 0.0F, 2.5F, 7.25F},
                        2);
}
