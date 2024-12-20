cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0167 OLD)

set(PATCH_VERSION "3" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 1.0.${PATCH_VERSION})


project(DuplicatesFinder VERSION ${PROJECT_VERSION})

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включение папки include
include_directories(include)

if(WIN32)
    set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows")
    set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
    set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
endif()

find_package(Boost COMPONENTS filesystem program_options REQUIRED)


# Создание исполняемого файла
add_executable(DuplicatesFinder main.cpp)
target_include_directories(DuplicatesFinder PRIVATE ${CMAKE_BINARY_DIR})
# Линковка с библиотеками Boost
target_link_libraries(DuplicatesFinder PRIVATE Boost::filesystem Boost::program_options)

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
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" 
        COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK 
        INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
    set(CPACK_GENERATOR ZIP)
elseif(UNIX)
    set(CPACK_GENERATOR DEB)
endif()

include(CPack)
