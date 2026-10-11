#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <utility>
#include <vector>

namespace merge_detail {

template <typename T>
class ElementReader {
public:
    explicit ElementReader(const std::filesystem::path& path)
        : stream_(path, std::ios::binary) {
        fetch();
    }

    bool valid() const { return valid_; }
    const T& current() const { return current_; }

    void fetch() {
        valid_ = static_cast<bool>(stream_.read(reinterpret_cast<char*>(&current_), sizeof(T)));
    }

private:
    std::ifstream stream_;
    T current_{};
    bool valid_{false};
};

template <typename T>
void write_element(std::ofstream& out, const T& value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template <typename T>
void write_vector(std::ofstream& out, const std::vector<T>& values) {
    for (const T& value : values) {
        write_element(out, value);
    }
}

template <typename T>
std::vector<T> read_fixed_run(ElementReader<T>& source, std::size_t run_size) {
    std::vector<T> run;
    run.reserve(run_size);
    while (run.size() < run_size && source.valid()) {
        run.push_back(source.current());
        source.fetch();
    }
    return run;
}

template <typename T>
std::vector<T> read_natural_run(ElementReader<T>& source) {
    std::vector<T> run;
    if (!source.valid()) {
        return run;
    }
    run.push_back(source.current());
    source.fetch();
    while (source.valid() && !(source.current() < run.back())) {
        run.push_back(source.current());
        source.fetch();
    }
    return run;
}

template <typename T>
void write_merged(std::ofstream& out, std::vector<T> left, std::vector<T> right) {
    std::size_t i = 0;
    std::size_t j = 0;
    while (i < left.size() && j < right.size()) {
        if (right[j] < left[i]) {
            write_element(out, right[j]);
            ++j;
        } else {
            write_element(out, left[i]);
            ++i;
        }
    }
    for (; i < left.size(); ++i) {
        write_element(out, left[i]);
    }
    for (; j < right.size(); ++j) {
        write_element(out, right[j]);
    }
}

inline void remove_if_exists(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

inline void replace_file(const std::filesystem::path& source, const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
    std::filesystem::rename(source, path, ec);
    if (ec) {
        std::filesystem::copy_file(source, path,
                                   std::filesystem::copy_options::overwrite_existing, ec);
        std::filesystem::remove(source, ec);
    }
}

inline bool file_is_non_empty(const std::filesystem::path& path) {
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    return !ec && size > 0;
}

}
