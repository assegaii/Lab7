cmake_minimum_required(VERSION 3.16)

# Установка политики CMake
if (POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
if (POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

# Установка версии проекта
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 1.0.${PATCH_VERSION})
project(DuplicatesFinder VERSION ${PROJECT_VERSION})

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройка Boost для MinGW
if(MINGW)
    set(BOOST_ROOT "$ENV{MSYSTEM_PREFIX}")
    set(Boost_USE_STATIC_LIBS ON)
endif()

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# Поиск библиотек Boost
find_package(Boost REQUIRED COMPONENTS system filesystem program_options)

# Проверка, что Boost найден
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "Boost not found. Please install Boost and set BOOST_ROOT.")
endif()

# Добавление исполняемого файла
add_executable(DuplicatesFinder main.cpp)

# Линковка Boost
target_link_libraries(DuplicatesFinder PRIVATE Boost::filesystem Boost::program_options)

# Настройка имени выходного файла для Windows
if(WIN32)
    set_target_properties(DuplicatesFinder PROPERTIES OUTPUT_NAME "DuplicatesFinder-${PROJECT_VERSION}-Windows")
endif()

# Опции компилятора
target_compile_options(DuplicatesFinder PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
)

# Установка проекта
install(TARGETS DuplicatesFinder DESTINATION bin)

# Настройка CPack для создания пакетов
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "dmitryanons@gmail.com")

# Генераторы пакетов для Windows и Unix
if(WIN32)
    set(CPACK_GENERATOR ZIP)
    set(CPACK_SOURCE_GENERATOR ZIP)
elseif(UNIX)
    set(CPACK_GENERATOR DEB)
    set(CPACK_SOURCE_GENERATOR TGZ)
endif()

include(CPack)
