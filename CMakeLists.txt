cmake_minimum_required(VERSION 2.8)
set(FASTHOPCROFT_VERSION_MAJOR 1)
set(FASTHOPCROFT_VERSION_MINOR 3)

project(FastHopcroft)
configure_file(src/config.h.in config.h)
find_package(Boost 1.40 REQUIRED)
include_directories(
	"${Boost_INCLUDE_DIR}"
	"${PROJECT_BINARY_DIR}")

add_executable(FastHopcroft
	"src/main.cpp")
	
install(TARGETS FastHopcroft DESTINATION bin)

export(PACKAGE FastHopcroft)
# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${FASTHOPCROFT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${FASTHOPCROFT_VERSION_MINOR}")
include(CPack)

if (MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE        "/MD /Ox /Ot /Gr /GL /D NDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Ox /Ot /Gr /GL /D NDEBUG /Zi")
endif(MSVC)