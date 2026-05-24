#pragma once

#include "StringAlgorithms.hpp"
#include "StringGenerator.hpp"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

struct TestConfig {
    std::size_t minSize = 100;
    std::size_t maxSize = 3000;
    std::size_t step = 100;
    int repetitions = 10;
};

struct Measurement {
    std::string algorithm;
    std::string dataset;
    std::size_t size = 0;
    int repetitions = 0;
    double meanMicroseconds = 0.0;
    double medianMicroseconds = 0.0;
    double standardDeviationMicroseconds = 0.0;
    std::uint64_t charComparisons = 0;
    std::uint64_t charAccesses = 0;
    bool sortedCorrectly = false;
};

class StringSortTester {
public:
    StringSortTester(StringGenerator::Datasets datasets, TestConfig config);

    void runAll(std::ostream& progress);
    void saveCsv(const std::string& path) const;
    const std::vector<Measurement>& results() const noexcept;

private:
    StringGenerator::Datasets datasets_;
    TestConfig config_;
    std::vector<Measurement> results_;

    Measurement benchmark(const StringAlgorithms::AlgorithmSpec& algorithm,
                          const std::string& datasetName,
                          const std::vector<std::string>& source,
                          std::size_t size) const;
};
