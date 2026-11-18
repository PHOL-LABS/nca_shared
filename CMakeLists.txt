cmake_minimum_required(VERSION 3.15)

set(NCA_SHARED_SOURCES
    src/nca_protocol.c
    src/nca_interactive.c
    src/nca_topics.c
    src/nca_device.c)

if(COMMAND idf_component_register)
    idf_component_register(SRCS ${NCA_SHARED_SOURCES} INCLUDE_DIRS include)
    return()
endif()

set(NCA_SHARED_TOP_LEVEL OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(nca_shared VERSION 1.0.0 LANGUAGES C CXX)
    set(NCA_SHARED_TOP_LEVEL ON)
endif()

add_library(nca_shared STATIC ${NCA_SHARED_SOURCES})
add_library(nca::shared ALIAS nca_shared)
target_include_directories(nca_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(nca_shared PUBLIC c_std_99)
set_target_properties(nca_shared PROPERTIES C_EXTENSIONS OFF)
set_target_properties(nca_shared PROPERTIES EXPORT_NAME shared)

option(NCA_SHARED_BUILD_TESTS "Build nca_shared Catch2 tests" ${NCA_SHARED_TOP_LEVEL})
option(NCA_SHARED_FETCH_CATCH2 "Fetch Catch2 when it is not installed" ON)

if(NCA_SHARED_BUILD_TESTS)
    include(CTest)
    find_package(Catch2 3 QUIET)
    if(NOT Catch2_FOUND AND NCA_SHARED_FETCH_CATCH2)
        include(FetchContent)
        FetchContent_Declare(Catch2
            URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.7.1.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
        FetchContent_MakeAvailable(Catch2)
    endif()
    if(NOT TARGET Catch2::Catch2WithMain)
        message(FATAL_ERROR "Catch2 v3 is required for NCA_SHARED_BUILD_TESTS")
    endif()

    add_executable(nca_shared_tests
        tests/test_protocol.cpp
        tests/test_topics.cpp
        tests/test_device.cpp)
    target_compile_features(nca_shared_tests PRIVATE cxx_std_17)
    target_link_libraries(nca_shared_tests PRIVATE nca::shared Catch2::Catch2WithMain)
    include(Catch)
    catch_discover_tests(nca_shared_tests)
endif()

if(NCA_SHARED_TOP_LEVEL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        cmake/nca_sharedConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/nca_sharedConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nca_shared)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/nca_sharedConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
    install(TARGETS nca_shared EXPORT nca_shared_targets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT nca_shared_targets FILE nca_sharedTargets.cmake
        NAMESPACE nca:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nca_shared)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/nca_sharedConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/nca_sharedConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nca_shared)
endif()
