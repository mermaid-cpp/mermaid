#[=======================================================================[.rst:
# simple helper for vcpkg's canvas_ity.
# tries to use proper CONFIG package, otherwise tries to
# find the main canvas_ity.hpp header.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# ``canvas_ity::canvas_ity`` - if found in Conan package or the "canvas_ity.hpp"
#                              was found.
#
#]=======================================================================]
find_package(canvas_ity CONFIG QUIET)
if(NOT canvas_ity_FOUND)
	find_path(CANVAS_ITY_INCLUDE_DIRS "canvas_ity.hpp")
	add_library(canvas_ity INTERFACE)
	target_include_directories(canvas_ity INTERFACE ${CANVAS_ITY_INCLUDE_DIRS})
	add_library(canvas_ity::canvas_ity ALIAS canvas_ity)
	set(canvas_ity_FOUND TRUE)
endif()

