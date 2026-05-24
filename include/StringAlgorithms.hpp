#pragma once

#include "Metrics.hpp"

#include <string>
#include <vector>

namespace StringAlgorithms {

using SortFunction = void (*)(std::vector<std::string>&, Metrics&);

struct AlgorithmSpec {
    std::string name;
    SortFunction sort;
};

void quickSort(std::vector<std::string>& values, Metrics& metrics);
void mergeSort(std::vector<std::string>& values, Metrics& metrics);
void stringQuickSort(std::vector<std::string>& values, Metrics& metrics);
void lcpMergeSort(std::vector<std::string>& values, Metrics& metrics);
void msdRadixSort(std::vector<std::string>& values, Metrics& metrics);
void hybridMsdRadixSort(std::vector<std::string>& values, Metrics& metrics);

std::vector<AlgorithmSpec> allAlgorithms();

} // namespace StringAlgorithms
