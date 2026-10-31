# SPDX-FileCopyrightText: 2026 mermaid-cpp contributors <https://github.com/mermaid-cpp/mermaid>
#
# SPDX-License-Identifier: MIT

#[=======================================================================[.rst:
mermaid-log.cmake
-----------------

Contains some little helpers for `message()` and log capatibilities
(e.g. CMAKE_MESSAGE_INDENT log prefix).

#]=======================================================================]

set(MERMAID_LOG_MESSAGE_INDENT "mermaid-cpp: " CACHE STRING "Allow overridr mermaid-cpp message log prefix, e.g., to integrate into another log facility/log category")

# Wraps cmake `message()` with mermaid log prefix.
macro(mermaid_log)
	list(APPEND CMAKE_MESSAGE_INDENT "${MERMAID_LOG_MESSAGE_INDENT}")
	message(${ARGV})	
	list(POP_BACK CMAKE_MESSAGE_INDENT)
endmacro()
