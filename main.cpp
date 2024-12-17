#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp> 
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

// Функция для вычисления CRC32 хеша
std::string compute_crc32(const std::string &data) {
    boost::crc_32_type crc32;
    crc32.process_bytes(data.data(), data.size());
    std::stringstream ss;
    ss << std::hex << crc32.checksum();
    return ss.str();
}

void process_files(const std::vector<fs::path>& directories, 
                   const std::vector<fs::path>& exclude_dirs,
                   unsigned int scan_depth, size_t block_size, 
                   size_t min_size) {

    std::unordered_map<std::string, std::vector<std::string>> file_hashes;

    // Для каждого файла в указанной директории
    for (const auto& dir : directories) {
        if (!fs::exists(dir) || !fs::is_directory(dir)) continue;

        for (fs::recursive_directory_iterator it(dir), end; it != end; ++it) {
            if (scan_depth == 0 && it.depth() > 0) continue;  // Пропускаем файлы с глубиной больше 0

            if (fs::is_directory(it->path()) && std::find(exclude_dirs.begin(), exclude_dirs.end(), it->path()) != exclude_dirs.end())
                continue;  // Пропускаем исключенные директории

            if (fs::is_regular_file(it->path())) {
                if (fs::file_size(it->path()) < min_size) continue;  // Пропускаем файлы меньше минимального размера

                std::ifstream file(it->path().string(), std::ios::binary);
                std::vector<std::string> file_hashes_blocks;
                std::string block;

                while (file) {
                    block.resize(block_size, '\0');  // Заполняем пустыми байтами, если файл короче блока
                    file.read(&block[0], block_size);
                    if (file.gcount() == 0) break;

                    std::string block_hash = compute_crc32(block);
                    file_hashes_blocks.push_back(block_hash);
                }

                if (!file_hashes_blocks.empty()) {
                    std::string file_hash = boost::algorithm::join(file_hashes_blocks, "");
                    file_hashes[file_hash].push_back(it->path().string());
                }
            }
        }
    }

    // Выводим дубликаты
    for (const auto& pair : file_hashes) {
        if (pair.second.size() > 1) {
            for (const auto& file : pair.second) {
                std::cout << file << std::endl;
            }
            std::cout << std::endl;  // Пустая строка между группами дубликатов
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        po::options_description desc("Options");
        desc.add_options()
            ("help,h", "Produce help message")
            ("directories,d", po::value<std::vector<fs::path>>()->multitoken(), "Directories to scan")
            ("exclude,x", po::value<std::vector<fs::path>>()->multitoken(), "Directories to exclude from scanning")
            ("depth,p", po::value<unsigned int>()->default_value(1), "Scan depth (0 - no subdirectories)")
            ("block-size,s", po::value<size_t>()->default_value(512), "Block size for hashing")
            ("min-size,m", po::value<size_t>()->default_value(1), "Minimum file size to consider (in bytes)")
            ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }

        if (!vm.count("directories")) {
            std::cerr << "Error: No directories specified for scanning!" << std::endl;
            return 1;
        }

        if (!vm.count("exclude")) {
            std::cerr << "Error: No exclude directories specified!" << std::endl;
            return 1;
        }

        std::vector<fs::path> directories = vm["directories"].as<std::vector<fs::path>>();
        std::vector<fs::path> exclude_dirs = vm["exclude"].as<std::vector<fs::path>>();
        unsigned int scan_depth = vm["depth"].as<unsigned int>();
        size_t block_size = vm["block-size"].as<size_t>();
        size_t min_size = vm["min-size"].as<size_t>();

        process_files(directories, exclude_dirs, scan_depth, block_size, min_size);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
