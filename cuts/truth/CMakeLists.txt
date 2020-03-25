add_subdirectory(targets)

#Set up a component library to force the plugin-loading code to detect these files when main() is built.
add_library(truthCuts OBJECT Cut.cpp IsAntineutrino.cpp IsNeutrino.cpp MuonMomentum.cpp Q3Range.cpp TrackAngle.cpp Apothem.cpp Q0Range.cpp EAvailRange.cpp IsCC.cpp IsNC.cpp MuonZMomentum.cpp)

install(FILES Cut.h Helicity.h MuonMomentum.h Q3Range.h TrackAngle.h Apothem.h Q0Range.h EAvailRange.h Current.h MuonZMomentum.h DESTINATION include)
