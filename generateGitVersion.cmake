#File: generateGitVersion.cmake
#Brief: CMake script to generate gitVersion.cpp.  I need a script to force this file to always be regenerated.
#       It's a cmake script so that it doesn't depend on platform.
#Author: Andrew Olivier aolivier@ur.rochester.edu

execute_process(COMMAND git rev-parse --verify HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_COMMIT_HASH)
execute_process(COMMAND git status --porcelain WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_UNCOMMITTED_CHANGES)
configure_file(gitVersion.cpp.in gitVersion.cpp @ONLY)
