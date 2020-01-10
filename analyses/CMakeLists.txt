add_subdirectory(signal)
add_subdirectory(sideband)
add_subdirectory(background)
#add_subdirectory(multiplicity)

add_library(analyses OBJECT MuonMomentum.cpp)
