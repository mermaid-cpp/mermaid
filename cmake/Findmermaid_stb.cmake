#[=======================================================================[.rst:
Findmermaid_stb.cmake
---------------------

Wrapper over conan/vcpkg packages.
At this moment (2024-12-17) conan and vcpkg exports
two different packages which differ the first letter's case
(conan exports stb-config.cmake while vcpkg generates FindStb.cmake).
As Windows doesn't distinct the upper and lower case of these files
I have to use the another facade name for my own find-module.

Cache variables
^^^^^^^^^^^^^^^

``mermaid_stb_INCLUDE_DIR``
  The directory containing the core 'stb_image.h' header file.

Imported targets
^^^^^^^^^^^^^^^^

``stb::stb``
  An INTERFACE target if we found vcpkg-like module or Conan
  package declared target. Otherwise, it will be created using
  the found system stb include path.

Result variables
^^^^^^^^^^^^^^^^

``mermaid_stb_FOUND``
  Indicates that stb has been found.

#]=======================================================================]

if(mermaid_stb_INSIDE)   # avoid recursion
	set(Stb_FOUND OFF)
	set(stb_FOUND OFF)
	set(mermaid_stb_FOUND OFF)
endif()

set(mermaid_stb_INSIDE ON)
set(_backup_cmake_module_path ${CMAKE_MODULE_PATH})
list(POP_BACK CMAKE_MODULE_PATH)
find_package(Stb QUIET) # Windows doesn't distinct lower and Capital cases here!
if(Stb_FOUND)
	# vcpkg way
	add_library(stb_lib INTERFACE)
	target_include_directories(stb_lib INTERFACE ${Stb_INCLUDE_DIR})
	add_library(stb::stb ALIAS stb_lib)
	set(Stb_FOUND TRUE)
	set(stb_FOUND TRUE)
	set(mermaid_stb_FOUND TRUE)
else()
	find_package(stb CONFIG QUIET)
	if(NOT stb_FOUND)
		find_path(mermaid_stb_INCLUDE_DIR
			NAMES stb_image.h
			PATH_SUFFIXES stb
			DOC "Directory containing stb_image.h"
			REQUIRED
		)
		cmake_path(GET mermaid_stb_INCLUDE_DIR ROOT_PATH mermaid_stb_INCLUDE_DIRS)
		add_library(mermaid_system_stb_stb INTERFACE)
		target_include_directories(mermaid_system_stb_stb INTERFACE ${mermaid_stb_INCLUDE_DIRS})
		add_library(stb::stb ALIAS mermaid_system_stb_stb)
		message(STATUS "Use system stb")
		set(stb_FOUND TRUE)
	elseif(NOT TARGET stb::stb)
		message(FATAL_ERROR "find_package(stb CONFIG) did not make target stb::stb")
	endif()
	set(mermaid_stb_FOUND ${stb_FOUND})
endif()

set(mermaid_stb_INSIDE OFF)
set(CMAKE_MODULE_PATH ${_backup_cmake_module_path})
unset(_backup_cmake_module_path)
