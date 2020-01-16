cmake_minimum_required( VERSION 2.8.12 )

set( CMAKE_C_COMPILER "$ENV{GCC_DIR}/bin/gcc")
set( CMAKE_CXX_COMPILER "$ENV{GCC_DIR}/bin/g++")

project( Utilities )

#Add extra "modules" to CMake
include(CTest)

#Compiler flags
set( GCC_Flags_For_CXX "-std=c++1y -Wall" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_Flags_For_CXX}" )
set( CMAKE_CXX_FLAGS_DEBUG "-ggdb" )
set( CMAKE_CXX_FLAGS_RELEASE "-O2" )

#Let directories in this package see each other
include_directories( "${PROJECT_SOURCE_DIR}" )

#Find dependencies
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOT_SYS})
find_package(ROOT REQUIRED COMPONENTS RIO Net Tree Core Geom EG GenVector)
include(${ROOT_USE_FILE})

find_package(yaml-cpp 0.6.0 REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIR})
#find_package(BaseUnits REQUIRED) #TODO: Reformat the CMake system in BaseUnits.  I'm embedding it for now to get started.
find_package(PlotUtils REQUIRED)
include_directories(${PlotUtils_INCLUDE_DIR})
message("Included PlotUtils from ${PlotUtils_INCLUDE_DIR}")

add_subdirectory(analyses)
add_subdirectory(cuts)

#Build main executables
add_executable(ProcessAnaTuples ProcessAnaTuples.cxx $<TARGET_OBJECTS:analyses> $<TARGET_OBJECTS:truthCuts> $<TARGET_OBJECTS:truthTargets> $<TARGET_OBJECTS:recoCuts> $<TARGET_OBJECTS:recoTargets>)
#This TARGET_OBJECTS song and dance solves the problem of the compiler
#refusing to link in self-registering libraries!  I learned about it from
#https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/Object-Library

#Set up to link against the current PlotUtils installation
#Taken from https://stackoverflow.com/questions/8774593/cmake-link-to-external-library

#Build libraries that main executables depend on
add_subdirectory(units)
add_subdirectory(util)
add_subdirectory(evt)
add_subdirectory(config)
add_subdirectory(app)

target_link_libraries(ProcessAnaTuples ${ROOT_LIBRARIES} util evt signal sideband background support yaml-cpp app_utilities)

install(TARGETS ProcessAnaTuples DESTINATION bin)

configure_file(setup.sh.in setup.sh @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/setup.sh DESTINATION bin)
