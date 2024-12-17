cmake_minimum_required(VERSION 3.10)

set(PATCH_VERSION "3" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 1.0.${PATCH_VERSION})

project(DuplicatesFinder VERSION ${PROJECT_VERSION})

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для Debug:
set(CMAKE_CXX_FLAGS_DEBUG "-std=gnu++17")

# Для Release:
set(CMAKE_CXX_FLAGS_RELEASE "-std=gnu++17")

# Включение папки include
include_directories(include)

find_package(Boost COMPONENTS filesystem program_options crc REQUIRED)

# Создание исполняемого файла
add_executable(DuplicatesFinder main.cpp)

# Линковка с библиотеками Boost
target_link_libraries(DuplicatesFinder PRIVATE Boost::filesystem Boost::program_options Boost::crc)

# Установка файла
install(TARGETS DuplicatesFinder DESTINATION bin)

# Включаем тестирование
include(CTest)

target_compile_options(DuplicatesFinder PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
)

# Настройки CPack для генерации пакетов
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "dmitryanons@gmail.com")

if(WIN32)
    set_target_properties(DuplicatesFinder PROPERTIES 
        OUTPUT_NAME "DuplicatesFinder-${PROJECT_VERSION}-Windows"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    set(CPACK_GENERATOR ZIP)
elseif(UNIX)
    set(CPACK_GENERATOR DEB)
endif()

include(CPack)
