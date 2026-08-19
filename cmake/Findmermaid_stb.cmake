#[=======================================================================[.rst:
# Findmermaid_stb.cmake
# ---------------------
#
# Wrapper over conan/vcpkg packages.
# At this moment (2024-12-17) conan and vcpkg exports
# two different packages which differ the first letter's case
# (conan exports stb-config.cmake while vcpkg generates FindStb.cmake).
# As Windows doesn't distinct the upper and lower case of these files
# I have to use the another facade name for my own find-module.
#
# Input variables
# ^^^^^^^^^^^^^^^
#
# ``Stb_INCLUDE_DIR`` - would used as stb include dir (also exported from vcpkg).
#                       Untested for now, will be checked with distros' system stb.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# ``stb::stb`` - an INTERFACE target if we found vcpkg-like module with
#                Stb_INCLUDE_DIR variable. Otherwise assume Conan
#		 declares it.
#
#]=======================================================================]
if(mermaid_stb_INSIDE)   # avoid recursion
	set(Stb_FOUND OFF)
	set(mermaid_stb_FOUND OFF)
endif()
set(mermaid_stb_INSIDE ON)
list(POP_BACK CMAKE_MODULE_PATH)
find_package(Stb) # Windows doesn't distinct lower and Capital cases here!
if (Stb_FOUND)
	# vcpkg way
	add_library(stb_lib INTERFACE)
	target_include_directories(stb_lib INTERFACE ${Stb_INCLUDE_DIR})
	add_library(stb::stb ALIAS stb_lib)
	set(stb_FOUND TRUE)
	set(mermaid_stb_FOUND TRUE)
else()
	find_package(stb CONFIG REQUIRED)
	if(NOT TARGET stb::stb)
		message(FATAL_ERROR "find_package(stb CONFIG) did not make target stb::stb")
	endif()
	set(mermaid_stb_FOUND ${stb_FOUND})
endif()

set(mermaid_stb_INSIDE OFF)
