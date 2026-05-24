#include "StringSortTester.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <ostream>
#include <stdexcept>

StringSortTester::StringSortTester(StringGenerator::Datasets datasets, TestConfig config)
    : datasets_(std::move(datasets)), config_(config) {
    if (config_.minSize == 0 || config_.step == 0 || config_.minSize > config_.maxSize ||
        config_.repetitions <= 0) {
        throw std::invalid_argument("Некорректная конфигурация тестирования");
    }
    for (const auto& [name, data] : datasets_) {
        if (data.size() < config_.maxSize) {
            throw std::invalid_argument("Набор данных '" + name + "' короче максимального тестового размера");
        }
    }
}

Measurement StringSortTester::benchmark(const StringAlgorithms::AlgorithmSpec& algorithm,
                                        const std::string& datasetName,
                                        const std::vector<std::string>& source,
                                        std::size_t size) const {
    std::vector<std::string> sample(source.begin(), source.begin() + static_cast<std::ptrdiff_t>(size));
    std::vector<std::string> expected = sample;
    std::sort(expected.begin(), expected.end());

    std::vector<double> elapsed;
    elapsed.reserve(static_cast<std::size_t>(config_.repetitions));
    Metrics referenceMetrics;
    for (int run = 0; run < config_.repetitions; ++run) {
        std::vector<std::string> working = sample;
        Metrics metrics;
        const auto begin = std::chrono::steady_clock::now();
        algorithm.sort(working, metrics);
        const auto end = std::chrono::steady_clock::now();
        if (working != expected) {
            throw std::runtime_error("Некорректный результат алгоритма " + algorithm.name + " на наборе " + datasetName);
        }
        if (run == 0) {
            referenceMetrics = metrics;
        } else if (metrics.charComparisons != referenceMetrics.charComparisons ||
                   metrics.charAccesses != referenceMetrics.charAccesses) {
            throw std::runtime_error("Недетерминированные значения метрик у алгоритма " + algorithm.name);
        }
        elapsed.push_back(std::chrono::duration<double, std::micro>(end - begin).count());
    }

    const double sum = std::accumulate(elapsed.begin(), elapsed.end(), 0.0);
    const double mean = sum / static_cast<double>(elapsed.size());
    std::vector<double> sortedElapsed = elapsed;
    std::sort(sortedElapsed.begin(), sortedElapsed.end());
    const std::size_t middle = sortedElapsed.size() / 2;
    const double median = sortedElapsed.size() % 2 == 0
                              ? (sortedElapsed[middle - 1] + sortedElapsed[middle]) / 2.0
                              : sortedElapsed[middle];
    double squareSum = 0.0;
    for (double value : elapsed) {
        const double deviation = value - mean;
        squareSum += deviation * deviation;
    }
    const double standardDeviation = std::sqrt(squareSum / static_cast<double>(elapsed.size()));

    return {algorithm.name, datasetName, size, config_.repetitions, mean, median,
            standardDeviation, referenceMetrics.charComparisons,
            referenceMetrics.charAccesses, true};
}

void StringSortTester::runAll(std::ostream& progress) {
    results_.clear();
    const auto algorithms = StringAlgorithms::allAlgorithms();
    for (const auto& algorithm : algorithms) {
        progress << "Измерение работы алгоритма " << algorithm.name << "...\n";
        for (const auto& [datasetName, data] : datasets_) {
            for (std::size_t size = config_.minSize; size <= config_.maxSize; size += config_.step) {
                results_.push_back(benchmark(algorithm, datasetName, data, size));
            }
        }
    }
}

void StringSortTester::saveCsv(const std::string& path) const {
    const std::filesystem::path outputPath(path);
    if (outputPath.has_parent_path()) {
        std::filesystem::create_directories(outputPath.parent_path());
    }
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Не удалось открыть файл для записи результатов: " + path);
    }
    output << "algorithm,dataset,size,repetitions,mean_us,median_us,stddev_us,char_comparisons,char_accesses,sorted_ok\n";
    output << std::fixed << std::setprecision(3);
    for (const Measurement& item : results_) {
        output << item.algorithm << ',' << item.dataset << ',' << item.size << ','
               << item.repetitions << ',' << item.meanMicroseconds << ','
               << item.medianMicroseconds << ',' << item.standardDeviationMicroseconds << ','
               << item.charComparisons << ',' << item.charAccesses << ','
               << (item.sortedCorrectly ? "true" : "false") << '\n';
    }
}

const std::vector<Measurement>& StringSortTester::results() const noexcept {
    return results_;
}
