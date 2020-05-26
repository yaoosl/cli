cmake_minimum_required(VERSION 2.8.12)
project(ys_cli)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MACOSX_RPATH 1)

# Enable multithreaded compilation in MSVC
if (MSVC)
  add_definitions(/MP)
endif()

# Configure output paths
set(ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)

# Include Paths
include_directories("${PROJECT_SOURCE_DIR}/include/tclap-1.2.2/include")

# Source Files
file(GLOB ys_cli_src 
    "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_BINARY_DIR}/src/git_sha1.cpp"
)

# Get the local git revision hash and put it into a header we can use
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
configure_file("${PROJECT_SOURCE_DIR}/cmake/git_sha1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/src/git_sha1.cpp" @ONLY)
list(APPEND ys_cli_src "${CMAKE_CURRENT_BINARY_DIR}/src/git_sha1.cpp")


# Add packages to project
# find_package(Threads)


option(YS_BUILD_CLI "BUILD CLI" ON)


if (YS_BUILD_CLI)
    add_executable(ys_cli ${ys_cli_src})
    target_link_libraries(ys_cli ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${ST_CXXFS_LIBS})
    target_include_directories(ys_cli PUBLIC ${PROJECT_SOURCE_DIR}/src)
endif ()