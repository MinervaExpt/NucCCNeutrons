cmake_minimum_required( VERSION 2.8.12 )

project( NucCCNeutrons )

#Add extra "modules" to CMake
include(CTest)
include(AddFileDependencies)

#Compiler flags
set( GCC_Flags_For_CXX "-std=c++1y -Wall" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_Flags_For_CXX}" )
set( CMAKE_CXX_FLAGS_DEBUG "-ggdb" )
set( CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" )

#Add a PROFiling CMAKE_BUILD_TYPE
set( CMAKE_CXX_FLAGS_PROF "-ggdb -pg -DNDEBUG" )
set( CMAKE_C_FLAGS_PROF "-ggdb -pg -DNDEBUG" )
set( CMAKE_EXE_LINKER_FLAGS_PROF "-ggdb -pg -DNDEBUG" )
set( CMAKE_SHARED_LINKER_FLAGS_PROF "-ggdb -pg -DNDEBUG" )

#Tell this package where it is installed and version control status
add_definitions(-DINSTALL_DIR="${CMAKE_INSTALL_PREFIX}/")

#add_custom_command(OUTPUT gitVersion.h COMMAND ${CMAKE_COMMAND} -P generateGitVersion.cmake DEPENDS gitVersion.h.in WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMENT "Pulling together git version information...")
add_custom_target(generateGitVersion COMMAND ${CMAKE_COMMAND} -P generateGitVersion.cmake DEPENDS gitVersion.h.in WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMENT "Pulling together git version information...")
add_file_dependencies(ProcessAnaTuples.cxx ${CMAKE_CURRENT_SOURCE_DIR}/gitVersion.h)

#Let directories in this package see each other
include_directories( "${PROJECT_SOURCE_DIR}" )

#Find dependencies
list( APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED COMPONENTS RIO Net Tree Core Geom EG GenVector Minuit2 Minuit OPTIONAL_COMPONENTS Cintex) #TODO: Remove CintEx if ROOT version > 5.34

if(${ROOT_VERSION} VERSION_LESS 6 AND NOT ${ROOT_Cintex_FOUND})
  MESSAGE(FATAL_ERROR "Cintex is optional except when it's not.  ROOT 6 has Reflex "
                      "support built in, so it doesn't have a separate component "
                      "for Cintex.  Cintex was an experimental feature in ROOT 5, "
                      "so I have to require it as a component there.  You appear to "
                      "be using ${ROOT_VERSION}, so I can't let you get away with "
                      "skipping out on Cintex support!  I need Cintex to load metadata "
                      "about MnvH1D and MnvH2D at runtime.")
endif()

if(${ROOT_VERSION} VERSION_LESS 6)
  MESSAGE("I need Cintex on this platform, so making sure to compile against it...")
else()
  MESSAGE("I don't need Cintex on this platform, so skipping it...")
  add_definitions(-DNCINTEX)
endif()

include(${ROOT_USE_FILE})

find_package(yaml-cpp 0.6.0 REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIR})

#find_package(BaseUnits REQUIRED) #TODO: Reformat the CMake system in BaseUnits.  I'm embedding it for now to get started.

find_package(PlotUtils REQUIRED)
include_directories(${PlotUtils_INCLUDE_DIR})
message("Included PlotUtils from ${PlotUtils_INCLUDE_DIR}")

find_package(UnfoldUtils REQUIRED)
include_directories(${UnfoldUtils_INCLUDE_DIR})
message("Included UnfoldUtils from ${UnfoldUtils_INCLUDE_DIR}")

#Some OBJECT libraries need this
add_subdirectory(util)

#Set up OBJECT libraries for plugins first
add_subdirectory(analyses)
add_subdirectory(cuts)
add_subdirectory(reweighters)
add_subdirectory(fiducials)
add_subdirectory(systematics)

#Build main executables
add_executable(ProcessAnaTuples ProcessAnaTuples.cxx $<TARGET_OBJECTS:studies> $<TARGET_OBJECTS:truthCuts> $<TARGET_OBJECTS:truthTargets> $<TARGET_OBJECTS:recoCuts> $<TARGET_OBJECTS:recoTargets> $<TARGET_OBJECTS:fiducials> $<TARGET_OBJECTS:systematics> $<TARGET_OBJECTS:reweighters>)
#This TARGET_OBJECTS song and dance solves the problem of the compiler
#refusing to link in self-registering libraries!  I learned about it from
#https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/Object-Library
add_executable(ExtractCrossSection ExtractCrossSection.cpp)
add_executable(SwapSysUnivWithCV SwapSysUnivWithCV.cpp)
add_executable(FitSidebands FitSidebands.cpp)

#Build libraries that main executables depend on
add_subdirectory(units)
add_subdirectory(evt)
add_subdirectory(config)
add_subdirectory(app)

add_dependencies(ProcessAnaTuples generateGitVersion)
target_link_libraries(ProcessAnaTuples ${ROOT_LIBRARIES} util evt analysesBase support yaml-cpp app)
target_link_libraries(ExtractCrossSection ${ROOT_LIBRARIES} PlotUtils UnfoldUtils)
target_link_libraries(SwapSysUnivWithCV ${ROOT_LIBRARIES} PlotUtils)
target_link_libraries(FitSidebands ${ROOT_LIBRARIES} PlotUtils)

install(TARGETS ProcessAnaTuples DESTINATION bin)
install(TARGETS ExtractCrossSection DESTINATION bin)
install(TARGETS SwapSysUnivWithCV DESTINATION bin)
install(TARGETS FitSidebands DESTINATION bin)

configure_file(setup.sh.in setup_${PROJECT_NAME}.sh @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/setup_${PROJECT_NAME}.sh DESTINATION bin)
