cmake_minimum_required(VERSION 3.14)
project(vermell VERSION 1.0.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VERMELL_HARDENING_FLAGS "-fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE")

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -pedantic -Wpedantic -Wnon-virtual-dtor -Wnull-dereference -Wstrict-overflow=5 -Wmissing-include-dirs -Wstrict-aliasing=2 -O3 -g -fsanitize=address ${VERMELL_HARDENING_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -pedantic -Wpedantic -Wnon-virtual-dtor -Wnull-dereference -Wstrict-overflow=5 -Wmissing-include-dirs -Wstrict-aliasing=2 -O3 ${VERMELL_HARDENING_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-pie -Wl,-z,relro,-z,now")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-pie -Wl,-z,relro,-z,now")

file(GLOB_RECURSE SOURCES
        "${CMAKE_SOURCE_DIR}/sources/*.cpp"
)

file(GLOB_RECURSE HEADERS
        "${CMAKE_SOURCE_DIR}/include/vermell/*.h"
)
# Shared when -DBUILD_SHARED_LIBS=ON (Debian), static otherwise (GitHub CI default).
add_library(vermell ${SOURCES} ${HEADERS})
set_target_properties(vermell PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
target_include_directories(vermell PUBLIC "${CMAKE_SOURCE_DIR}/include")

set(LIBURING_CUSTOM_INCLUDE_HINT "/usr/include")
set(LIBURING_CUSTOM_LIB_HINT     "/usr/lib")

find_path(LIBURING_INCLUDE_DIR
    NAMES liburing.h
    HINTS ${LIBURING_CUSTOM_INCLUDE_HINT}
)

find_library(LIBURING_LIBRARY
    NAMES uring
    HINTS ${LIBURING_CUSTOM_LIB_HINT}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUring
    REQUIRED_VARS LIBURING_LIBRARY LIBURING_INCLUDE_DIR
)

if(LibUring_FOUND)
    target_link_libraries(vermell PRIVATE ${LIBURING_LIBRARY})
    target_include_directories(vermell PRIVATE ${LIBURING_INCLUDE_DIR})
    target_compile_definitions(vermell PRIVATE DURING)

    message(STATUS "--------------------------------------------")
    message(STATUS "LibUring Discovery Status:")
    message(STATUS "  LibUring_FOUND:       ${LibUring_FOUND}")
   message(STATUS "  LIBURING_INCLUDE_DIR: ${LIBURING_INCLUDE_DIR}")
    message(STATUS "  LIBURING_LIBRARY:     ${LIBURING_LIBRARY}")
    message(STATUS "  Compiler Flag Added:  -DDURING")
    message(STATUS "--------------------------------------------")
else()
    message(STATUS "liburing could not be found. Falling back to kernel paths")
endif()

option(TESTING "Enable testing" OFF)
if (TESTING)

    enable_testing()
    add_executable(main tests/test_cases.cpp)
    target_link_libraries(
            main
            vermell
    )

    find_package(GTest REQUIRED)
    message(STATUS "using local GTest: in case of error verify your installation")

    target_link_libraries(main GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(main)

    find_package(CURL REQUIRED)
    message(STATUS "using local curl: in case of error with .so.4 verify with `curl --version`")

    target_link_libraries(main CURL::libcurl)

endif ()

include(GNUInstallDirs)
install(TARGETS vermell
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/vermell" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES "${CMAKE_SOURCE_DIR}/README.md" DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/vermell)


if(NOT TARGET uninstall AND EXISTS "${CMAKE_SOURCE_DIR}/cmake/uninstall.cmake.in")
    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/uninstall.cmake.in"
        "${CMAKE_BINARY_DIR}/uninstall.cmake"
        IMMEDIATE @ONLY
    )
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/uninstall.cmake"
    )
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "libvermell")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}" CACHE STRING "Version of the generated package (overridable with -DCPACK_PACKAGE_VERSION=...)")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "User-friendly and compact C++20 Web Framework")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/vermellcc/vermell")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kevin Bohorquez - kevin@bohorquez.cc")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
include(CPack)
