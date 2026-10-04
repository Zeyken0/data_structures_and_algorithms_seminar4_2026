#pragma once

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

namespace merge_detail {

template <typename T>
bool read_element(std::ifstream& in, T& value) {
    return static_cast<bool>(in.read(reinterpret_cast<char*>(&value), sizeof(T)));
}

template <typename T>
void write_element(std::ofstream& out, const T& value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template <typename T>
std::vector<T> read_block(std::ifstream& in, std::size_t count) {
    std::vector<T> block;
    block.reserve(count);
    T value;
    while (block.size() < count && read_element(in, value)) {
        block.push_back(value);
    }
    return block;
}

template <typename T>
void write_vector(std::ofstream& out, const std::vector<T>& values) {
    for (const T& value : values) {
        write_element(out, value);
    }
}

template <typename T>
void sort_block(std::vector<T>& block) {
    std::sort(block.begin(), block.end());
}

template <typename T>
void copy_rest(std::ifstream& in, std::ofstream& out) {
    std::vector<T> block = read_block<T>(in, 4096);
    while (!block.empty()) {
        write_vector(out, block);
        block = read_block<T>(in, 4096);
    }
}

template <typename T>
std::vector<T> merge_vectors(const std::vector<T>& left, const std::vector<T>& right) {
    std::vector<T> merged;
    merged.reserve(left.size() + right.size());
    std::size_t i = 0;
    std::size_t j = 0;
    while (i < left.size() && j < right.size()) {
        if (left[i] <= right[j]) {
            merged.push_back(left[i++]);
        } else {
            merged.push_back(right[j++]);
        }
    }
    while (i < left.size()) merged.push_back(left[i++]);
    while (j < right.size()) merged.push_back(right[j++]);
    return merged;
}

template <typename T>
void merge_files(const std::filesystem::path& left,
                 const std::filesystem::path& right,
                 const std::filesystem::path& out_path) {
    std::ifstream in_left(left, std::ios::binary);
    std::ifstream in_right(right, std::ios::binary);
    std::ofstream out(out_path, std::ios::binary | std::ios::trunc);

    T a{};
    T b{};
    bool has_a = read_element(in_left, a);
    bool has_b = read_element(in_right, b);

    while (has_a && has_b) {
        if (a <= b) {
            write_element(out, a);
            has_a = read_element(in_left, a);
        } else {
            write_element(out, b);
            has_b = read_element(in_right, b);
        }
    }
    if (has_a) {
        write_element(out, a);
        copy_rest<T>(in_left, out);
    }
    if (has_b) {
        write_element(out, b);
        copy_rest<T>(in_right, out);
    }
}

template <typename T>
void merge_file_with_block(const std::filesystem::path& file,
                           const std::vector<T>& block,
                           const std::filesystem::path& out_path) {
    std::ifstream in(file, std::ios::binary);
    std::ofstream out(out_path, std::ios::binary | std::ios::trunc);

    T value{};
    bool has_value = read_element(in, value);
    std::size_t i = 0;

    while (has_value && i < block.size()) {
        if (value <= block[i]) {
            write_element(out, value);
            has_value = read_element(in, value);
        } else {
            write_element(out, block[i]);
            ++i;
        }
    }
    if (has_value) {
        write_element(out, value);
        copy_rest<T>(in, out);
    }
    if (i < block.size()) {
        write_vector(out, std::vector<T>(block.begin() + static_cast<std::ptrdiff_t>(i), block.end()));
    }
}

template <typename T>
void append_file(const std::filesystem::path& from, const std::filesystem::path& to) {
    std::ifstream in(from, std::ios::binary);
    std::ofstream out(to, std::ios::binary | std::ios::app);
    std::vector<T> block = read_block<T>(in, 4096);
    while (!block.empty()) {
        write_vector(out, block);
        block = read_block<T>(in, 4096);
    }
}

template <typename T>
std::size_t count_runs(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    T prev{};
    T cur{};
    if (!read_element(in, prev)) return 0;

    std::size_t runs = 1;
    while (read_element(in, cur)) {
        if (cur < prev) ++runs;
        prev = cur;
    }
    return runs;
}

template <typename T>
struct ElementStream {
    std::ifstream in;
    T current{};
    bool valid = false;

    explicit ElementStream(const std::filesystem::path& path) : in(path, std::ios::binary) {
        valid = read_element(in, current);
    }

    void advance() {
        valid = read_element(in, current);
    }
};

template <typename T>
std::vector<T> read_run(ElementStream<T>& src) {
    std::vector<T> run;
    if (!src.valid) return run;

    run.push_back(src.current);
    src.advance();
    while (src.valid && !(src.current < run.back())) {
        run.push_back(src.current);
        src.advance();
    }
    return run;
}

inline void replace_file(const std::filesystem::path& from, const std::filesystem::path& to) {
    std::error_code ec;
    std::filesystem::remove(to, ec);
    std::filesystem::rename(from, to);
}

inline void remove_if_exists(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

inline std::filesystem::path make_path(const std::filesystem::path& dir,
                                       const std::string& prefix,
                                       std::size_t index) {
    return dir / (prefix + std::to_string(index) + ".bin");
}

}
