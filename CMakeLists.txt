cmake_minimum_required(VERSION 2.8)
set(FASTHOPCROFT_VERSION_MAJOR 1)
set(FASTHOPCROFT_VERSION_MINOR 3)

project(FastHopcroft)
configure_file(src/config.h.in config.h)
find_package(Boost 1.54 REQUIRED)
include_directories(
	"${Boost_INCLUDE_DIR}"
	"${PROJECT_BINARY_DIR}")

link_directories("${Boost_LIBRARY_DIRS}")
	
if (MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE        "/MD /Ox /Ot /Gr /GL /D NDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Ox /Ot /Gr /GL /D NDEBUG /Zi")
endif(MSVC)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
	set(CMAKE_CXX_COMPILER_IS_CLANG TRUE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "-std=c++11")
    set(CMAKE_CXX_FLAGS_RELEASE "-O4")
endif()
	
add_subdirectory(src/convert_nfa_dfa)
add_subdirectory(src/generate_dfa)
add_subdirectory(src/generate_nfa)
add_subdirectory(src/minimize_brzozowski)
add_subdirectory(src/minimize_hopcroft)
add_subdirectory(src/test)

#export(PACKAGE FastHopcroftExecutable)
# build a CPack driven installer package
#include (InstallRequiredSystemLibraries)
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
#set(CPACK_PACKAGE_VERSION_MAJOR "${FASTHOPCROFT_VERSION_MAJOR}")
#set(CPACK_PACKAGE_VERSION_MINOR "${FASTHOPCROFT_VERSION_MINOR}")
#include(CPack)

