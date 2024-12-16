cmake_minimum_required(VERSION 3.16)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 1.0.${PATCH_VERSION})

# Для Debug:
set(CMAKE_CXX_FLAGS_DEBUG "-static-libgcc -static-libstdc++ -std=gnu++17")

# Для Release:
set(CMAKE_CXX_FLAGS_RELEASE "-static-libgcc -static-libstdc++ -std=gnu++17")

project(DuplicatesFinder VERSION ${PROJECT_VERSION})

# Указываем путь к Boost в папке external
set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/external/boost")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")

# Ищем компоненты Boost, включая program_options, filesystem и crc
find_package(Boost REQUIRED COMPONENTS filesystem program_options crc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Создание исполняемого файла
add_executable(DuplicatesFinder main.cpp)

# Линковка с библиотеками Boost
target_link_libraries(DuplicatesFinder Boost::boost Boost::program_options Boost::filesystem Boost::crc)

install(TARGETS DuplicatesFinder DESTINATION bin)
include(CPack)


# Настройки для MSVC
if(MSVC)
    
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
    set_target_properties(DuplicatesFinder PROPERTIES 
        OUTPUT_NAME "DuplicatesFinder-${PROJECT_VERSION}-Windows"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    set(CPACK_GENERATOR ZIP)

# Настройки для UNIX
elseif(UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
    set(CPACK_GENERATOR DEB)
endif()

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT dmitryanons@gmail.com)

# Включаем тестирование
include(CTest)

# Настройки для пакета
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

