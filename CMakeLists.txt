cmake_minimum_required(VERSION 3.16)

if (POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 1.0.${PATCH_VERSION})

project(DuplicatesFinder VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/externals/boost")
include_directories("${BOOST_ROOT}")

set(Boost_LIBRARIES
    "${BOOST_ROOT}/stage/lib/libboost_filesystem.a"
    "${BOOST_ROOT}/stage/lib/libboost_system.a"
)

add_executable(DuplicatesFinder main.cpp)

target_link_libraries(DuplicatesFinder PRIVATE ${Boost_LIBRARIES})

if(WIN32)
    set_target_properties(DuplicatesFinder PROPERTIES OUTPUT_NAME "DuplicatesFinder-${PROJECT_VERSION}-Windows")
endif()

install(TARGETS DuplicatesFinder DESTINATION bin)


target_compile_options(DuplicatesFinder PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
)

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "dmitryanons@gmail.com")

if(WIN32)
    set(CPACK_GENERATOR ZIP) 
    set(CPACK_SOURCE_GENERATOR ZIP)
elseif(UNIX)
    set(CPACK_GENERATOR DEB) 
    set(CPACK_SOURCE_GENERATOR TGZ)
endif()

include(CPack)
