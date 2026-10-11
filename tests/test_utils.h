#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

inline std::filesystem::path make_temp_root() {
    return std::filesystem::temp_directory_path() / "external_sorting_tests";
}

struct TempDir {
    std::filesystem::path path;

    explicit TempDir(const std::string& name) : path(make_temp_root() / name) {
        std::filesystem::remove_all(path);
        std::filesystem::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }

    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    std::filesystem::path operator/(const std::string& child) const {
        return path / child;
    }
};

template <typename T>
void write_vector_to_file(const std::filesystem::path& path, const std::vector<T>& values) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    for (const T& value : values) {
        out.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }
}

template <typename T>
std::vector<T> read_vector_from_file(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::vector<T> values;
    T value;
    while (in.read(reinterpret_cast<char*>(&value), sizeof(T))) {
        values.push_back(value);
    }
    return values;
}

inline std::vector<int> make_random_ints(std::size_t count, std::uint32_t seed, int range = 1000) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(-range, range);
    std::vector<int> values(count);
    for (int& value : values) {
        value = dist(gen);
    }
    return values;
}

inline std::vector<double> make_random_doubles(std::size_t count, std::uint32_t seed) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> dist(-1000.0, 1000.0);
    std::vector<double> values(count);
    for (double& value : values) {
        value = dist(gen);
    }
    return values;
}

template <typename T>
std::vector<T> sorted_copy(std::vector<T> values) {
    std::sort(values.begin(), values.end());
    return values;
}

inline std::vector<int> make_alternating_values(std::size_t count) {
    std::vector<int> values;
    values.reserve(count);
    int low = 0;
    int high = static_cast<int>(count);
    while (values.size() < count) {
        values.push_back(high--);
        if (values.size() < count) {
            values.push_back(low++);
        }
    }
    return values;
}
