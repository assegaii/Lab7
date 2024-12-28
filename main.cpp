#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <boost/crc.hpp>
#include <boost/program_options.hpp> 
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <regex>
#include <set>

namespace fs = std::filesystem;
namespace po = boost::program_options;

// Функция для вычисления CRC32
uint32_t calculate_crc32(const std::string& data) {
    boost::crc_32_type crc;
    crc.process_bytes(data.data(), data.size());
    return crc.checksum();
}

// Функция для обработки файла и получения последовательности хэшей
std::vector<uint32_t> file_processing(const fs::path& filePath, size_t blockSize) {
    std::vector<uint32_t> hashSequence;
    std::ifstream file(filePath, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Cannot open file: " + filePath.string());
    }

    std::string buffer(blockSize, '\0');

    while (file.read(&buffer[0], blockSize) || file.gcount() > 0) {
        size_t bytesRead = static_cast<size_t>(file.gcount());
        buffer.resize(bytesRead);
        if (bytesRead < blockSize) {
            buffer.resize(blockSize, '\0');
        }
        uint32_t hash = calculate_crc32(buffer);
        hashSequence.push_back(hash);
    }

    return hashSequence;
}

// Функция для сравнения двух файлов по их хэшам
bool compare_hashes(const std::vector<uint32_t>& hashes1, const std::vector<uint32_t>& hashes2) {
    return hashes1 == hashes2;
}

void shouldProcessFile(const fs::directory_entry& entry, const std::vector<fs::path>& exclusions, size_t minSize, const std::regex& maskRegex, size_t blockSize, std::vector<std::pair<fs::path, std::vector<uint32_t>>>& hashVector) {
    if (entry.is_regular_file()) {
        if (std::find(exclusions.begin(), exclusions.end(), entry.path().parent_path()) != exclusions.end()) {
            return;
        }
        if (entry.file_size() < minSize) {
            return;
        }
        if (!std::regex_match(entry.path().filename().string(), maskRegex)) {
            return;
        }
        auto hashes = file_processing(entry.path(), blockSize);
        hashVector.emplace_back(entry.path(), hashes);
    }
}

void find_duplicates(const std::vector<fs::path>& directories, const std::vector<fs::path>& exclusions, size_t blockSize, size_t minSize, const std::regex& maskRegex, int scanLevel) {
    std::unordered_map<std::string, std::set<fs::path>> hashMap;
    std::vector<std::pair<fs::path, std::vector<uint32_t>>> hashVector;

    for (const auto& dir : directories) {
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            std::cerr << "Directory does not exist or is not a directory: " << dir << std::endl;
            continue;
        }

        if (scanLevel == 0) {
            for (const auto& entry : fs::directory_iterator(dir)) {
                shouldProcessFile(entry, exclusions, minSize, maskRegex, blockSize, hashVector);
            }
        } else {
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                shouldProcessFile(entry, exclusions, minSize, maskRegex, blockSize, hashVector);
            }
        }
    }

    for (size_t i = 0; i < hashVector.size(); ++i) {
        for (size_t j = i + 1; j < hashVector.size(); ++j) {
            if (compare_hashes(hashVector[i].second, hashVector[j].second)) {
                std::string hashKey(reinterpret_cast<const char*>(hashVector[i].second.data()), hashVector[i].second.size() * sizeof(uint32_t));
                hashMap[hashKey].insert(hashVector[i].first);
                hashMap[hashKey].insert(hashVector[j].first);
            }
        }
    }

    for (const auto& pair : hashMap) {
        std::cout << "Duplicates:\n";
        for (const auto& file : pair.second) {
            std::cout << file << std::endl;
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::vector<fs::path> directories;
    std::vector<fs::path> exclusions;
    size_t blockSize = 4096;
    size_t minSize = 1;
    int scanLevel = 1;
    std::string maskString = "*";

    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "Show help message")
            ("directories,d", po::value<std::vector<fs::path>>(&directories)->multitoken(), "Directories to scan")
            ("exclusions,e", po::value<std::vector<fs::path>>(&exclusions)->multitoken(), "Directories to exclude")
            ("block-size,b", po::value<size_t>(&blockSize)->default_value(4096), "Block size")
            ("min-size,m", po::value<size_t>(&minSize)->default_value(1), "Minimum file size")
            ("scan-level,l", po::value<int>(&scanLevel)->default_value(1), "Scan level (0 for current directory only, 1 for recursive)")
            ("mask,k", po::value<std::string>(&maskString)->default_value("*"), "File mask (e.g., *.txt)");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }

        std::regex star_regex("\\*");
        std::regex question_regex("\\?");
        maskString = "^" + std::regex_replace(maskString, star_regex, ".*");
        maskString = std::regex_replace(maskString, question_regex, ".");
        maskString += "$";

        std::regex maskRegex(maskString, std::regex_constants::icase);
        find_duplicates(directories, exclusions, blockSize, minSize, maskRegex, scanLevel);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
