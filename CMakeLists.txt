cmake_minimum_required(VERSION 3.20)
project(mermaid VERSION 0.0.1 LANGUAGES C CXX)

# CMake modules.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(mermaid-log)
include(GNUInstallDirs)

# Base options.
option(MERMAID_BUILD_TESTAPP "Build simple testapp" ON)
option(MERMAID_BUILD_TESTS "Enable tests" "${BUILD_TESTING}")
option(MERMAID_ENABLE_RENDERERS "Enable renderers (disable if: fuzz/parser tests/system build/custom renderer logic)" OFF)


# Deps.
if(MERMAID_ENABLE_RENDERERS)
	find_package(canvas_ity REQUIRED)
	find_package(mermaid_stb REQUIRED) # facade over conan/vcpkg/system stb installation
endif()

find_package(fmt REQUIRED)


# Main lib - parsers for Mermaid diagrams.
add_library(mermaid lib.cpp)
set_target_properties(mermaid PROPERTIES PUBLIC_HEADER "include/mermaid/mermaid.hpp")
target_compile_features(mermaid PUBLIC cxx_std_23)
target_include_directories(mermaid PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(mermaid PRIVATE fmt::fmt)

# Diagram renderers.
#

if(MERMAID_ENABLE_RENDERERS)
	add_library(mermaid-renderer renderer.cpp)
	target_compile_features(mermaid-renderer PUBLIC cxx_std_23)
	target_link_libraries(mermaid-renderer canvas_ity::canvas_ity)

	add_library(mermaid-renderer-console renderer-fmt-console.cpp)
	target_compile_features(mermaid-renderer-console PUBLIC cxx_std_23)
	target_link_libraries(mermaid-renderer-console fmt::fmt)

	add_library(mermaid-renderer-image renderer-stb-image.cpp)
	target_compile_features(mermaid-renderer-image PUBLIC cxx_std_23)
	target_link_libraries(mermaid-renderer-image fmt::fmt stb::stb)

	# link all specific renderers into the umbrella lib
	target_link_libraries(mermaid-renderer mermaid-renderer-image mermaid-renderer-console)
else()
	add_library(mermaid-renderer INTERFACE)
endif()


if(MERMAID_BUILD_TESTAPP)
	# testapp find fonts logic
	mermaid_log(STATUS "build testapp")

	mermaid_log(CHECK_START "find default font for testapp")
	mermaid_log(STATUS "Using font dir: ${MERMAID_FONTS_DIR}")

	# Find fonts for testapp renderer
	set(MERMAID_FONTS_DIR "${CMAKE_INSTALL_FULL_DATADIR}/fonts" CACHE PATH "Default fonts path")
	find_file(MERMAID_TESTAPP_TTF_FILENAME "DejaVuSans.ttf"
		HINTS "${MERMAID_FONTS_DIR}"
		"$ENV{WINDIR}/Fonts"
		"$ENV{HOME}/../usr/share/fonts/TTF" # Termux
		"${MERMAID_FONTS_DIR}/TTF")

	# Main testapp (PoC).
	add_executable(mermaid_testapp main.cpp)
	target_link_libraries(mermaid_testapp mermaid)
	if(MERMAID_ENABLE_RENDERERS)
		target_link_libraries(mermaid_testapp mermaid-renderer)
	else()
		target_compile_definitions(mermaid_testapp PRIVATE MERMAID_NO_RENDERERS=1)
		mermaid_log(STATUS "Info: testapp will be w/o actual renderers")
	endif()

	# add font if found
	if(MERMAID_TESTAPP_TTF_FILENAME)
		mermaid_log(CHECK_PASS "found at path ${MERMAID_TESTAPP_TTF_FILENAME}")
		if (MERMAID_ENABLE_RENDERERS)
			target_compile_definitions(mermaid-renderer PUBLIC MERMAID_TESTAPP_TTF_FILENAME="${MERMAID_TESTAPP_TTF_FILENAME}")
		endif()
	else()
		mermaid_log(CHECK_FAIL "DejaVuSans default testapp font did not found. The testapp won't render fonts properly")
	endif()

	if(MERMAID_BUILD_TESTS)
		enable_testing()
		add_test(NAME testapp COMMAND ${CMAKE_CURRENT_BUILD_DIR}/mermaid_testapp)
		set_tests_properties(testapp PROPERTIES LABELS "unit;testapp")
	endif()
endif()
