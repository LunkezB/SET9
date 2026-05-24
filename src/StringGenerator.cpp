#include "StringGenerator.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace {
const std::string kAlphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#%:;^&*()-";
}

StringGenerator::StringGenerator(std::uint64_t seed) : rng_(seed) {}

const std::string& StringGenerator::alphabet() {
    return kAlphabet;
}

std::string StringGenerator::randomString(std::size_t minLength, std::size_t maxLength) {
    std::uniform_int_distribution<std::size_t> lengthDistribution(minLength, maxLength);
    std::uniform_int_distribution<std::size_t> charDistribution(0, kAlphabet.size() - 1);

    const std::size_t length = lengthDistribution(rng_);
    std::string result;
    result.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        result.push_back(kAlphabet[charDistribution(rng_)]);
    }
    return result;
}

StringGenerator::Datasets StringGenerator::generate(std::size_t maxSize,
                                                     std::size_t minLength,
                                                     std::size_t maxLength) {
    if (maxSize == 0 || minLength == 0 || minLength > maxLength) {
        throw std::invalid_argument("Некорректные параметры генерации данных");
    }

    std::vector<std::string> random;
    random.reserve(maxSize);
    for (std::size_t i = 0; i < maxSize; ++i) {
        random.push_back(randomString(minLength, maxLength));
    }

    std::vector<std::string> reverse = random;
    std::sort(reverse.begin(), reverse.end());
    std::reverse(reverse.begin(), reverse.end());

    std::vector<std::string> nearlySorted = random;
    std::sort(nearlySorted.begin(), nearlySorted.end());
    for (std::size_t block = 0; block + 1 < nearlySorted.size(); block += 100) {
        const std::size_t blockEnd = std::min(block + 100, nearlySorted.size());
        if (blockEnd - block < 2) {
            continue;
        }
        std::uniform_int_distribution<std::size_t> position(block, blockEnd - 2);
        const std::size_t i = position(rng_);
        std::swap(nearlySorted[i], nearlySorted[i + 1]);
    }

    Datasets datasets;
    datasets.emplace("random", std::move(random));
    datasets.emplace("reverse", std::move(reverse));
    datasets.emplace("nearly_sorted", std::move(nearlySorted));

    return datasets;
}

void StringGenerator::saveCsv(const Datasets& datasets, const std::string& path) {
    const std::filesystem::path outputPath(path);
    if (outputPath.has_parent_path()) {
        std::filesystem::create_directories(outputPath.parent_path());
    }
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Не удалось открыть файл для записи наборов данных: " + path);
    }
    output << "dataset,index,value\n";
    for (const auto& [name, values] : datasets) {
        for (std::size_t i = 0; i < values.size(); ++i) {
            output << name << ',' << i << ',' << values[i] << '\n';
        }
    }
}
