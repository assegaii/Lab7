#include <boost/filesystem.hpp>
#include <boost/crc.hpp>
#include <boost/algorithm/string/join.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <sstream>  // Подключаем заголовочный файл для stringstream

namespace fs = boost::filesystem;

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

int main() {
    std::vector<std::string> directories;
    std::vector<std::string> exclude_dirs;
    unsigned int scan_depth;
    size_t block_size;
    size_t min_size;

    // Вводим директории для сканирования
    std::cout << "Enter directories to scan (separate by space): ";
    std::string dir_input;
    std::getline(std::cin, dir_input);
    std::istringstream dir_stream(dir_input);
    std::string dir;
    while (dir_stream >> dir) {
        directories.push_back(dir);
    }

    // Вводим исключенные директории
    std::cout << "Enter directories to exclude (separate by space, press enter if none): ";
    std::string exclude_input;
    std::getline(std::cin, exclude_input);
    std::istringstream exclude_stream(exclude_input);
    while (exclude_stream >> dir) {
        exclude_dirs.push_back(dir);
    }

    // Вводим глубину сканирования
    std::cout << "Enter scan depth (0 for no subdirectories, default is 1): ";
    std::string depth_input;
    std::getline(std::cin, depth_input);
    if (!depth_input.empty()) {
        scan_depth = std::stoi(depth_input);
    } else {
        scan_depth = 1;  // Значение по умолчанию
    }

    // Вводим размер блока для хеширования
    std::cout << "Enter block size for hashing (default is 512): ";
    std::string block_size_input;
    std::getline(std::cin, block_size_input);
    if (!block_size_input.empty()) {
        block_size = std::stoi(block_size_input);
    } else {
        block_size = 512;  // Значение по умолчанию
    }

    // Вводим минимальный размер файла
    std::cout << "Enter minimum file size to consider (in bytes, default is 1): ";
    std::string min_size_input;
    std::getline(std::cin, min_size_input);
    if (!min_size_input.empty()) {
        min_size = std::stoi(min_size_input);
    } else {
        min_size = 1;  // Значение по умолчанию
    }

    // Преобразуем строки директорий в тип fs::path
    std::vector<fs::path> dir_paths(directories.begin(), directories.end());
    std::vector<fs::path> exclude_paths(exclude_dirs.begin(), exclude_dirs.end());

    // Обрабатываем файлы
    process_files(dir_paths, exclude_paths, scan_depth, block_size, min_size);

    return 0;
}
