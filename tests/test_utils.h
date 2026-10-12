#pragma once

#include <filesystem>
#include <fstream>
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

