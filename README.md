# External Sorting Algorithms

Реализация трёх алгоритмов внешней сортировки на C++ с модульными тестами.

## Алгоритмы
- Two-Phase Merge Sort (двухфазная сортировка простым слиянием)
- One-Phase Merge Sort (однофазная сортировка простым слиянием)
- Natural Merge Sort (естественная сортировка)

## Стек
- C++17
- CMake 3.16+
- Catch2 v2 (vendored)
- clangd / Neovim

## Сборка
```bash
cmake -S . -B build
cmake --build build
```

## Запуск тестов
```bash
./build/tests/sorting_tests
```
