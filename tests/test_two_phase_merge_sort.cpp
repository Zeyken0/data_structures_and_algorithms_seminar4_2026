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
    REQUIRE(std::filesystem::is_empty(temp_dir));
}

}

TEST_CASE("TwoPhaseMergeSort: пустой файл", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_empty", {}, {}, 1);
}

TEST_CASE("TwoPhaseMergeSort: один элемент", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_single", {12}, {12}, 1);
}

TEST_CASE("TwoPhaseMergeSort: уже отсортированный", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_sorted",
                      {0, 2, 5, 6, 8, 12, 14, 18},
                      {0, 2, 5, 6, 8, 12, 14, 18},
                      2);
}

TEST_CASE("TwoPhaseMergeSort: обратный порядок", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_reversed",
                      {18, 14, 12, 8, 6, 5, 2, 0},
                      {0, 2, 5, 6, 8, 12, 14, 18},
                      2);
}

TEST_CASE("TwoPhaseMergeSort: тестовый массив", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_test_array",
                      {12, 6, 0, 14, 5, 18, 2, 8},
                      {0, 2, 5, 6, 8, 12, 14, 18},
                      1);
}

TEST_CASE("TwoPhaseMergeSort: много дубликатов", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_duplicates",
                      {12, 6, 0, 14, 5, 18, 2, 8, 12, 6, 0, 14, 5, 18, 2, 8},
                      {0, 0, 2, 2, 5, 5, 6, 6, 8, 8, 12, 12, 14, 14, 18, 18},
                      3);
}

TEST_CASE("TwoPhaseMergeSort: отрицательные числа", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_negative",
                      {-12, -6, 0, -14, -5, -18, -2, -8},
                      {-18, -14, -12, -8, -6, -5, -2, 0},
                      2);
}

TEST_CASE("TwoPhaseMergeSort: размер серии 1", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_unit_runs",
                      {18, 14, 12, 8, 6, 5, 2, 0},
                      {0, 2, 5, 6, 8, 12, 14, 18},
                      1);
}

TEST_CASE("TwoPhaseMergeSort: размер серии 3 (серий больше степени двойки)",
          "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_three_runs",
                      {12, 6, 0, 14, 5, 18, 2, 8},
                      {0, 2, 5, 6, 8, 12, 14, 18},
                      3);
}

TEST_CASE("TwoPhaseMergeSort: размер серии не делится нацело (хвост)",
          "[two_phase_merge_sort]") {
    SECTION("хвост из трёх элементов (серия 5)") {
        check_sorted<int>("two_phase_tail_three",
                          {12, 6, 0, 14, 5, 18, 2, 8},
                          {0, 2, 5, 6, 8, 12, 14, 18},
                          5);
    }
    SECTION("хвост из одного элемента (серия 7)") {
        check_sorted<int>("two_phase_tail_single",
                          {12, 6, 0, 14, 5, 18, 2, 8},
                          {0, 2, 5, 6, 8, 12, 14, 18},
                          7);
    }
}

TEST_CASE("TwoPhaseMergeSort: серия покрывает весь файл", "[two_phase_merge_sort]") {
    check_sorted<int>("two_phase_single_run",
                      {12, 6, 0, 14, 5, 18, 2, 8},
                      {0, 2, 5, 6, 8, 12, 14, 18},
                      8);
}

TEST_CASE("TwoPhaseMergeSort: два элемента, оба порядка", "[two_phase_merge_sort]") {
    SECTION("по возрастанию") {
        check_sorted<int>("two_phase_two_asc", {6, 12}, {6, 12}, 1);
    }
    SECTION("по убыванию") {
        check_sorted<int>("two_phase_two_desc", {12, 6}, {6, 12}, 1);
    }
}

TEST_CASE("TwoPhaseMergeSort: double", "[two_phase_merge_sort][double]") {
    SECTION("положительные дробные") {
        check_sorted<double>("two_phase_double_pos",
                             {6.0, 3.0, 0.0, 7.0, 2.5, 9.0, 1.0, 4.0},
                             {0.0, 1.0, 2.5, 3.0, 4.0, 6.0, 7.0, 9.0},
                             2);
    }
    SECTION("отрицательные и положительные") {
        check_sorted<double>("two_phase_double_mix",
                             {-6.0, 3.0, 0.0, -7.0, 2.5, -9.0, 1.0, 4.0},
                             {-9.0, -7.0, -6.0, 0.0, 1.0, 2.5, 3.0, 4.0},
                             2);
    }
    SECTION("пустой файл") {
        check_sorted<double>("two_phase_double_empty", {}, {}, 2);
    }
}

TEST_CASE("TwoPhaseMergeSort: float", "[two_phase_merge_sort][float]") {
    check_sorted<float>("two_phase_float",
                        {6.0F, 3.0F, 0.0F, 7.0F, 2.5F, 9.0F, 1.0F, 4.0F},
                        {0.0F, 1.0F, 2.5F, 3.0F, 4.0F, 6.0F, 7.0F, 9.0F},
                        2);
}
