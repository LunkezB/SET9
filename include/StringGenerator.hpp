#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <random>
#include <string>
#include <vector>

class StringGenerator {
public:
    using Datasets = std::map<std::string, std::vector<std::string>>;

    explicit StringGenerator(std::uint64_t seed = 20269999ULL);

    Datasets generate(std::size_t maxSize = 3000,
                      std::size_t minLength = 10,
                      std::size_t maxLength = 200);

    static void saveCsv(const Datasets& datasets, const std::string& path);
    static const std::string& alphabet();

private:
    std::mt19937_64 rng_;

    std::string randomString(std::size_t minLength, std::size_t maxLength);
};
