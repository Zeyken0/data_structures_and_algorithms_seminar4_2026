#include <catch.hpp>
#include "one_phase_merge_sort.h"
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
    one_phase_merge_sort<T>(input_file, temp_dir, run_size);

    REQUIRE(read_vector_from_file<T>(input_file) == expected);
    REQUIRE(std::filesystem::is_empty(temp_dir));
}

}

TEST_CASE("OnePhaseMergeSort: пустой файл", "[one_phase_merge_sort]") {
    check_sorted<int>("one_phase_empty", {}, {}, 4);
}

TEST_CASE("OnePhaseMergeSort: один элемент", "[one_phase_merge_sort]") {
    check_sorted<int>("one_phase_single", {42}, {42}, 4);
}

TEST_CASE("OnePhaseMergeSort: уже отсортированный", "[one_phase_merge_sort]") {
    check_sorted<int>("one_phase_sorted", {1, 2, 3, 4, 5}, {1, 2, 3, 4, 5}, 2);
}

TEST_CASE("OnePhaseMergeSort: обратный порядок", "[one_phase_merge_sort]") {
    check_sorted<int>("one_phase_reversed", {5, 4, 3, 2, 1}, {1, 2, 3, 4, 5}, 2);
}

TEST_CASE("OnePhaseMergeSort: произвольный порядок", "[one_phase_merge_sort]") {
    check_sorted<int>("one_phase_arbitrary",
                      {3, 1, 4, 1, 5, 9, 2, 6, 5, 3},
                      {1, 1, 2, 3, 3, 4, 5, 5, 6, 9},
                      3);
}

TEST_CASE("OnePhaseMergeSort: много дубликатов", "[one_phase_merge_sort]") {
    check_sorted<int>("one_phase_duplicates", {5, 1, 5, 1, 5, 1}, {1, 1, 1, 5, 5, 5}, 2);
}

TEST_CASE("OnePhaseMergeSort: отрицательные числа", "[one_phase_merge_sort]") {
    check_sorted<int>("one_phase_negative", {-1, -5, -3, -2, -4}, {-5, -4, -3, -2, -1}, 2);
}

TEST_CASE("OnePhaseMergeSort: размер серии 1", "[one_phase_merge_sort]") {
    check_sorted<int>("one_phase_unit_runs",
                      {9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
                      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                      1);
}

TEST_CASE("OnePhaseMergeSort: большой файл", "[one_phase_merge_sort]") {
    std::vector<int> input;
    std::vector<int> expected;
    for (int i = 99; i >= 0; --i) {
        input.push_back(i);
        expected.push_back(99 - i);
    }
    check_sorted<int>("one_phase_large", input, expected, 1024);
}

TEST_CASE("OnePhaseMergeSort: два элемента, оба порядка", "[one_phase_merge_sort]") {
    SECTION("по возрастанию") {
        check_sorted<int>("one_phase_two_asc", {1, 2}, {1, 2}, 1);
    }
    SECTION("по убыванию") {
        check_sorted<int>("one_phase_two_desc", {2, 1}, {1, 2}, 1);
    }
}

TEST_CASE("OnePhaseMergeSort: double", "[one_phase_merge_sort][double]") {
    SECTION("положительные дробные") {
        check_sorted<double>("one_phase_double_pos",
                             {3.5, 1.25, 2.75, 0.5, 4.0},
                             {0.5, 1.25, 2.75, 3.5, 4.0},
                             2);
    }
    SECTION("отрицательные и положительные") {
        check_sorted<double>("one_phase_double_mix",
                             {-1.5, 2.25, -3.75, 0.0, 1.5},
                             {-3.75, -1.5, 0.0, 1.5, 2.25},
                             2);
    }
    SECTION("пустой файл") {
        check_sorted<double>("one_phase_double_empty", {}, {}, 2);
    }
}
