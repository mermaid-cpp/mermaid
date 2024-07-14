cmake_minimum_required(VERSION 3.17)
project(mermaid VERSION 0.0.1 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(GNUInstallDirs)
find_package(canvas_ity REQUIRED)
find_package(mermaid_stb REQUIRED) # facade over conan/vcpkg/system stb installation
find_package(fmt REQUIRED)

option(MERMAID_BUILD_TESTAPP "Build simple testapp" ON)

# Main lib - parsers for Mermaid diagrams.
add_library(mermaid lib.cpp)
target_compile_features(mermaid PUBLIC cxx_std_23)
target_include_directories(mermaid PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Diagram renderers.
#

target_link_libraries(mermaid PRIVATE fmt::fmt)
add_library(mermaid-renderer-console renderer-fmt-console.cpp)
target_compile_features(mermaid-renderer-console PUBLIC cxx_std_23)
target_link_libraries(mermaid-renderer-console fmt::fmt)

add_library(mermaid-renderer-image renderer-stb-image.cpp)
target_compile_features(mermaid-renderer-image PUBLIC cxx_std_23)
target_link_libraries(mermaid-renderer-image fmt::fmt stb::stb)


if(MERMAID_BUILD_TESTAPP)
	# testapp find fonts logic
	list(APPEND CMAKE_MESSAGE_INDENT "mermaid-cpp: ")
	message(STATUS "build testapp")

	message(CHECK_START "find default font for testapp")
	message(STATUS "Using font dir: ${MERMAID_FONTS_DIR}")

	# Find fonts for testapp renderer
	set(MERMAID_FONTS_DIR "${CMAKE_INSTALL_FULL_DATADIR}/fonts" CACHE PATH "Default fonts path")
	find_file(MERMAID_TESTAPP_TTF_FILENAME "DejaVuSans.ttf"
		HINTS "${MERMAID_FONTS_DIR}"
		"$ENV{WINDIR}/Fonts"
		"$ENV{HOME}/../usr/share/fonts/TTF" # Termux
		"${MERMAID_FONTS_DIR}/TTF")

	# Main testapp (PoC).
	add_executable(mermaid_testapp main.cpp)
	target_link_libraries(mermaid_testapp mermaid mermaid-renderer-console mermaid-renderer-image canvas_ity::canvas_ity)

	# add font if found
	if(MERMAID_TESTAPP_TTF_FILENAME)
		message(CHECK_PASS "found at path ${MERMAID_TESTAPP_TTF_FILENAME}")
		target_compile_definitions(mermaid_testapp PUBLIC MERMAID_TESTAPP_TTF_FILENAME="${MERMAID_TESTAPP_TTF_FILENAME}")
	else()
		message(CHECK_FAIL "DejaVuSans default testapp font did not found. The testapp won't render fonts properly")
	endif()

	enable_testing()
	add_test(NAME testapp COMMAND ${CMAKE_CURRENT_BUILD_DIR}/mermaid_testapp)

	list(POP_BACK CMAKE_MESSAGE_INDENT)
endif()
