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
    REQUIRE(std::filesystem::is_empty(temp_dir));
}

}

TEST_CASE("NaturalMergeSort: пустой файл", "[natural_merge_sort]") {
    check_sorted<int>("natural_empty", {}, {});
}

TEST_CASE("NaturalMergeSort: один элемент", "[natural_merge_sort]") {
    check_sorted<int>("natural_single", {12}, {12});
}

TEST_CASE("NaturalMergeSort: уже отсортированный", "[natural_merge_sort]") {
    check_sorted<int>("natural_sorted",
                      {0, 2, 5, 6, 8, 12, 14, 18},
                      {0, 2, 5, 6, 8, 12, 14, 18});
}

TEST_CASE("NaturalMergeSort: обратный порядок", "[natural_merge_sort]") {
    check_sorted<int>("natural_reversed",
                      {18, 14, 12, 8, 6, 5, 2, 0},
                      {0, 2, 5, 6, 8, 12, 14, 18});
}

TEST_CASE("NaturalMergeSort: тестовый массив", "[natural_merge_sort]") {
    check_sorted<int>("natural_test_array",
                      {12, 6, 0, 14, 5, 18, 2, 8},
                      {0, 2, 5, 6, 8, 12, 14, 18});
}

TEST_CASE("NaturalMergeSort: много дубликатов", "[natural_merge_sort]") {
    check_sorted<int>("natural_duplicates",
                      {12, 6, 0, 14, 5, 18, 2, 8, 12, 6, 0, 14, 5, 18, 2, 8},
                      {0, 0, 2, 2, 5, 5, 6, 6, 8, 8, 12, 12, 14, 14, 18, 18});
}

TEST_CASE("NaturalMergeSort: отрицательные числа", "[natural_merge_sort]") {
    check_sorted<int>("natural_negative",
                      {-12, -6, 0, -14, -5, -18, -2, -8},
                      {-18, -14, -12, -8, -6, -5, -2, 0});
}

TEST_CASE("NaturalMergeSort: две чередующиеся серии", "[natural_merge_sort]") {
    check_sorted<int>("natural_two_runs",
                      {0, 2, 5, 6, 18, 8, 12, 14},
                      {0, 2, 5, 6, 8, 12, 14, 18});
}

TEST_CASE("NaturalMergeSort: чередующиеся серии длиной 2", "[natural_merge_sort]") {
    check_sorted<int>("natural_pairs",
                      {0, 18, 2, 14, 5, 12, 6, 8},
                      {0, 2, 5, 6, 8, 12, 14, 18});
}

TEST_CASE("NaturalMergeSort: склейка первой и последней серии", "[natural_merge_sort]") {
    check_sorted<int>("natural_glue_runs",
                      {6, 8, 12, 0, 2, 18, 5, 14},
                      {0, 2, 5, 6, 8, 12, 14, 18});
}

TEST_CASE("NaturalMergeSort: серия больше прочих", "[natural_merge_sort]") {
    check_sorted<int>("natural_big_run",
                      {18, 14, 0, 2, 5, 6, 8, 12},
                      {0, 2, 5, 6, 8, 12, 14, 18});
}

TEST_CASE("NaturalMergeSort: серия занимает почти весь файл", "[natural_merge_sort]") {
    check_sorted<int>("natural_almost_one_run",
                      {2, 5, 6, 8, 12, 14, 18, 0},
                      {0, 2, 5, 6, 8, 12, 14, 18});
}

TEST_CASE("NaturalMergeSort: два элемента, оба порядка", "[natural_merge_sort]") {
    SECTION("по возрастанию") {
        check_sorted<int>("natural_two_asc", {6, 12}, {6, 12});
    }
    SECTION("по убыванию") {
        check_sorted<int>("natural_two_desc", {12, 6}, {6, 12});
    }
}

TEST_CASE("NaturalMergeSort: double", "[natural_merge_sort][double]") {
    SECTION("положительные дробные") {
        check_sorted<double>("natural_double_pos",
                             {6.0, 3.0, 0.0, 7.0, 2.5, 9.0, 1.0, 4.0},
                             {0.0, 1.0, 2.5, 3.0, 4.0, 6.0, 7.0, 9.0});
    }
    SECTION("отрицательные и положительные") {
        check_sorted<double>("natural_double_mix",
                             {-6.0, 3.0, 0.0, -7.0, 2.5, -9.0, 1.0, 4.0},
                             {-9.0, -7.0, -6.0, 0.0, 1.0, 2.5, 3.0, 4.0});
    }
    SECTION("пустой файл") {
        check_sorted<double>("natural_double_empty", {}, {});
    }
}

TEST_CASE("NaturalMergeSort: float", "[natural_merge_sort][float]") {
    check_sorted<float>("natural_float",
                        {6.0F, 3.0F, 0.0F, 7.0F, 2.5F, 9.0F, 1.0F, 4.0F},
                        {0.0F, 1.0F, 2.5F, 3.0F, 4.0F, 6.0F, 7.0F, 9.0F});
}
