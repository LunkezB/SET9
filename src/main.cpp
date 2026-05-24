#include "StringGenerator.hpp"
#include "StringSortTester.hpp"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
struct Options {
    std::string outputDirectory = "results";
    std::uint64_t seed = 20269999ULL;
    int repetitions = 10;
};

void printHelp(const char* program) {
    std::cout << "Использование: " << program << " [параметры]\n"
              << "  --output <каталог>      Каталог для результатов (по умолчанию: results)\n"
              << "  --seed <число>          Seed генератора (по умолчанию: 20269999)\n"
              << "  --repetitions <число>   Число измеряемых запусков для случая (по умолчанию: 10)\n"
              << "  --help                  Показать эту справку\n";
}

Options parseOptions(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help") {
            printHelp(argv[0]);
            std::exit(0);
        } else if (arg == "--output" && i + 1 < argc) {
            options.outputDirectory = argv[++i];
        } else if (arg == "--seed" && i + 1 < argc) {
            options.seed = std::stoull(argv[++i]);
        } else if (arg == "--repetitions" && i + 1 < argc) {
            options.repetitions = std::stoi(argv[++i]);
        } else {
            throw std::invalid_argument("Неизвестный или неполный параметр командной строки: " + arg);
        }
    }
    if (options.repetitions <= 0) {
        throw std::invalid_argument("Число повторений должно быть положительным");
    }
    return options;
}
} // namespace

int main(int argc, char** argv) {
    try {
        const Options options = parseOptions(argc, argv);
        std::filesystem::create_directories(options.outputDirectory);

        StringGenerator generator(options.seed);
        auto datasets = generator.generate(3000, 10, 200);
        StringGenerator::saveCsv(datasets, options.outputDirectory + "/datasets.csv");

        TestConfig config;
        config.repetitions = options.repetitions;
        StringSortTester tester(std::move(datasets), config);
        tester.runAll(std::cout);
        tester.saveCsv(options.outputDirectory + "/results.csv");

        std::cout << "Завершено: " << tester.results().size() << " измерений записано в "
                  << options.outputDirectory << "/results.csv\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
