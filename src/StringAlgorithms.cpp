#include "StringAlgorithms.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace StringAlgorithms {
namespace {

constexpr std::size_t kAlphabetSize = 74;
constexpr std::string_view kInputAlphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#%:;^&*()-";

struct CompareResult {
    int order = 0;
    std::size_t lcp = 0;
};

CompareResult compareFrom(const std::string& left,
                          const std::string& right,
                          std::size_t knownEqualPrefix,
                          Metrics& metrics) {
    std::size_t i = knownEqualPrefix;
    while (i < left.size() && i < right.size()) {
        ++metrics.charComparisons;
        metrics.charAccesses += 2;
        const auto a = static_cast<unsigned char>(left[i]);
        const auto b = static_cast<unsigned char>(right[i]);
        if (a < b) {
            return {-1, i};
        }
        if (a > b) {
            return {1, i};
        }
        ++i;
    }
    if (left.size() < right.size()) {
        return {-1, i};
    }
    if (left.size() > right.size()) {
        return {1, i};
    }
    return {0, i};
}

bool lessFull(const std::string& left, const std::string& right, Metrics& metrics) {
    return compareFrom(left, right, 0, metrics).order < 0;
}

std::ptrdiff_t partitionQuickSort(std::vector<std::string>& values,
                                 std::ptrdiff_t left,
                                 std::ptrdiff_t right,
                                 Metrics& metrics) {
    const std::ptrdiff_t pivotIndex = left + (right - left) / 2;
    const std::string pivot = values[static_cast<std::size_t>(pivotIndex)];
    std::swap(values[static_cast<std::size_t>(pivotIndex)],
              values[static_cast<std::size_t>(right)]);
    std::ptrdiff_t boundary = left;
    for (std::ptrdiff_t i = left; i < right; ++i) {
        if (lessFull(values[static_cast<std::size_t>(i)], pivot, metrics)) {
            std::swap(values[static_cast<std::size_t>(i)],
                      values[static_cast<std::size_t>(boundary)]);
            ++boundary;
        }
    }
    std::swap(values[static_cast<std::size_t>(boundary)],
              values[static_cast<std::size_t>(right)]);
    return boundary;
}

void quickSortImpl(std::vector<std::string>& values,
                   std::ptrdiff_t left,
                   std::ptrdiff_t right,
                   Metrics& metrics) {
    if (left >= right) {
        return;
    }
    const std::ptrdiff_t pivot = partitionQuickSort(values, left, right, metrics);
    quickSortImpl(values, left, pivot - 1, metrics);
    quickSortImpl(values, pivot + 1, right, metrics);
}

void mergeSortImpl(std::vector<std::string>& values,
                   std::vector<std::string>& buffer,
                   std::size_t left,
                   std::size_t right,
                   Metrics& metrics) {
    if (right - left <= 1) {
        return;
    }
    const std::size_t middle = left + (right - left) / 2;
    mergeSortImpl(values, buffer, left, middle, metrics);
    mergeSortImpl(values, buffer, middle, right, metrics);

    std::size_t i = left;
    std::size_t j = middle;
    std::size_t out = left;
    while (i < middle && j < right) {
        if (lessFull(values[j], values[i], metrics)) {
            buffer[out++] = values[j++];
        } else {
            buffer[out++] = values[i++];
        }
    }
    while (i < middle) {
        buffer[out++] = values[i++];
    }
    while (j < right) {
        buffer[out++] = values[j++];
    }
    for (std::size_t k = left; k < right; ++k) {
        values[k] = std::move(buffer[k]);
    }
}

int characterAt(const std::string& value, std::size_t depth, Metrics& metrics) {
    if (depth >= value.size()) {
        return -1;
    }
    ++metrics.charAccesses;
    return static_cast<unsigned char>(value[depth]);
}

void stringQuickSortImpl(std::vector<std::string>& values,
                         std::ptrdiff_t left,
                         std::ptrdiff_t right,
                         std::size_t depth,
                         Metrics& metrics) {
    if (left >= right) {
        return;
    }

    const std::ptrdiff_t pivotIndex = left + (right - left) / 2;
    const int pivot = characterAt(values[static_cast<std::size_t>(pivotIndex)], depth, metrics);
    std::ptrdiff_t less = left;
    std::ptrdiff_t greater = right;
    std::ptrdiff_t i = left;

    while (i <= greater) {
        const int current = characterAt(values[static_cast<std::size_t>(i)], depth, metrics);
        if (current >= 0 && pivot >= 0) {
            ++metrics.charComparisons;
        }
        if (current < pivot) {
            std::swap(values[static_cast<std::size_t>(less++)],
                      values[static_cast<std::size_t>(i++)]);
        } else if (current > pivot) {
            std::swap(values[static_cast<std::size_t>(i)],
                      values[static_cast<std::size_t>(greater--)]);
        } else {
            ++i;
        }
    }

    stringQuickSortImpl(values, left, less - 1, depth, metrics);
    if (pivot >= 0) {
        stringQuickSortImpl(values, less, greater, depth + 1, metrics);
    }
    stringQuickSortImpl(values, greater + 1, right, depth, metrics);
}

void lcpMerge(std::vector<std::string>& values,
              std::vector<std::size_t>& lcp,
              std::vector<std::string>& buffer,
              std::vector<std::size_t>& bufferLcp,
              std::size_t left,
              std::size_t middle,
              std::size_t right,
              Metrics& metrics) {
    std::size_t i = left;
    std::size_t j = middle;
    std::size_t out = left;
    std::size_t leftToLast = 0;
    std::size_t rightToLast = 0;

    const CompareResult first = compareFrom(values[i], values[j], 0, metrics);
    if (first.order <= 0) {
        buffer[out] = values[i++];
        bufferLcp[out++] = 0;
        if (i < middle) {
            leftToLast = lcp[i];
        }
        rightToLast = first.lcp;
    } else {
        buffer[out] = values[j++];
        bufferLcp[out++] = 0;
        leftToLast = first.lcp;
        if (j < right) {
            rightToLast = lcp[j];
        }
    }

    while (i < middle && j < right) {
        const std::size_t hl = leftToLast;
        const std::size_t hr = rightToLast;
        if (hl > hr) {
            buffer[out] = values[i++];
            bufferLcp[out++] = hl;
            if (i < middle) {
                leftToLast = lcp[i];
            }
        } else if (hr > hl) {
            buffer[out] = values[j++];
            bufferLcp[out++] = hr;
            if (j < right) {
                rightToLast = lcp[j];
            }
        } else {
            const CompareResult compared = compareFrom(values[i], values[j], hl, metrics);
            if (compared.order <= 0) {
                buffer[out] = values[i++];
                bufferLcp[out++] = hl;
                rightToLast = compared.lcp;
                if (i < middle) {
                    leftToLast = lcp[i];
                } else {
                }
            } else {
                buffer[out] = values[j++];
                bufferLcp[out++] = hr;
                leftToLast = compared.lcp;
                if (j < right) {
                    rightToLast = lcp[j];
                } else {
                }
            }
        }
    }

    if (i < middle) {
        buffer[out] = values[i++];
        bufferLcp[out++] = leftToLast;
        while (i < middle) {
            buffer[out] = values[i];
            bufferLcp[out] = lcp[i];
            ++i;
            ++out;
        }
    } else if (j < right) {
        buffer[out] = values[j++];
        bufferLcp[out++] = rightToLast;
        while (j < right) {
            buffer[out] = values[j];
            bufferLcp[out] = lcp[j];
            ++j;
            ++out;
        }
    }

    for (std::size_t k = left; k < right; ++k) {
        values[k] = std::move(buffer[k]);
        lcp[k] = bufferLcp[k];
    }
}

void lcpMergeSortImpl(std::vector<std::string>& values,
                      std::vector<std::size_t>& lcp,
                      std::vector<std::string>& buffer,
                      std::vector<std::size_t>& bufferLcp,
                      std::size_t left,
                      std::size_t right,
                      Metrics& metrics) {
    if (right - left <= 1) {
        return;
    }
    const std::size_t middle = left + (right - left) / 2;
    lcpMergeSortImpl(values, lcp, buffer, bufferLcp, left, middle, metrics);
    lcpMergeSortImpl(values, lcp, buffer, bufferLcp, middle, right, metrics);
    lcpMerge(values, lcp, buffer, bufferLcp, left, middle, right, metrics);
}

const std::array<int, 256>& rankTable() {
    static const std::array<int, 256> ranks = [] {
        std::array<int, 256> result{};
        result.fill(-1);
        std::string sorted(kInputAlphabet);
        std::sort(sorted.begin(), sorted.end());
        sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
        if (sorted.size() != kAlphabetSize) {
            throw std::logic_error("Алфавит должен содержать ровно 74 уникальных символа");
        }
        for (std::size_t i = 0; i < sorted.size(); ++i) {
            result[static_cast<unsigned char>(sorted[i])] = static_cast<int>(i + 1);
        }
        return result;
    }();
    return ranks;
}

int bucketAt(const std::string& value, std::size_t depth, Metrics& metrics) {
    if (depth >= value.size()) {
        return 0;
    }
    ++metrics.charAccesses;
    const int bucket = rankTable()[static_cast<unsigned char>(value[depth])];
    if (bucket < 0) {
        throw std::invalid_argument("Во входных данных найден символ вне заданного алфавита");
    }
    return bucket;
}

void msdSortImpl(std::vector<std::string>& values,
                 std::vector<std::string>& buffer,
                 std::size_t left,
                 std::size_t right,
                 std::size_t depth,
                 bool useQuickSwitch,
                 Metrics& metrics) {
    const std::size_t length = right - left;
    if (length <= 1) {
        return;
    }
    if (useQuickSwitch && length < kAlphabetSize) {
        stringQuickSortImpl(values,
                            static_cast<std::ptrdiff_t>(left),
                            static_cast<std::ptrdiff_t>(right - 1),
                            depth,
                            metrics);
        return;
    }

    std::array<std::size_t, kAlphabetSize + 1> frequencies{};
    std::vector<int> buckets(length);
    for (std::size_t offset = 0; offset < length; ++offset) {
        const int bucket = bucketAt(values[left + offset], depth, metrics);
        buckets[offset] = bucket;
        ++frequencies[static_cast<std::size_t>(bucket)];
    }

    std::array<std::size_t, kAlphabetSize + 2> starts{};
    for (std::size_t bucket = 0; bucket <= kAlphabetSize; ++bucket) {
        starts[bucket + 1] = starts[bucket] + frequencies[bucket];
    }
    auto next = starts;
    for (std::size_t offset = 0; offset < length; ++offset) {
        const auto bucket = static_cast<std::size_t>(buckets[offset]);
        buffer[left + next[bucket]++] = std::move(values[left + offset]);
    }
    for (std::size_t i = left; i < right; ++i) {
        values[i] = std::move(buffer[i]);
    }
    for (std::size_t bucket = 1; bucket <= kAlphabetSize; ++bucket) {
        const std::size_t bucketLeft = left + starts[bucket];
        const std::size_t bucketRight = left + starts[bucket + 1];
        if (bucketRight - bucketLeft > 1) {
            msdSortImpl(values, buffer, bucketLeft, bucketRight, depth + 1,
                        useQuickSwitch, metrics);
        }
    }
}

} // namespace

void quickSort(std::vector<std::string>& values, Metrics& metrics) {
    if (!values.empty()) {
        quickSortImpl(values, 0, static_cast<std::ptrdiff_t>(values.size() - 1), metrics);
    }
}

void mergeSort(std::vector<std::string>& values, Metrics& metrics) {
    std::vector<std::string> buffer(values.size());
    mergeSortImpl(values, buffer, 0, values.size(), metrics);
}

void stringQuickSort(std::vector<std::string>& values, Metrics& metrics) {
    if (!values.empty()) {
        stringQuickSortImpl(values, 0, static_cast<std::ptrdiff_t>(values.size() - 1), 0, metrics);
    }
}

void lcpMergeSort(std::vector<std::string>& values, Metrics& metrics) {
    std::vector<std::string> buffer(values.size());
    std::vector<std::size_t> lcp(values.size(), 0);
    std::vector<std::size_t> bufferLcp(values.size(), 0);
    lcpMergeSortImpl(values, lcp, buffer, bufferLcp, 0, values.size(), metrics);
}

void msdRadixSort(std::vector<std::string>& values, Metrics& metrics) {
    std::vector<std::string> buffer(values.size());
    msdSortImpl(values, buffer, 0, values.size(), 0, false, metrics);
}

void hybridMsdRadixSort(std::vector<std::string>& values, Metrics& metrics) {
    std::vector<std::string> buffer(values.size());
    msdSortImpl(values, buffer, 0, values.size(), 0, true, metrics);
}

std::vector<AlgorithmSpec> allAlgorithms() {
    return {
        {"quick_sort", quickSort},
        {"merge_sort", mergeSort},
        {"string_quick_sort", stringQuickSort},
        {"lcp_merge_sort", lcpMergeSort},
        {"msd_radix_sort", msdRadixSort},
        {"hybrid_msd_radix_sort", hybridMsdRadixSort},
    };
}

} // namespace StringAlgorithms
