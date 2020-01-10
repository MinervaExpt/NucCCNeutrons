#Set up a component library to force the plugin-loading code to detect these files when main() is built.
add_library(truthCuts OBJECT Cut.cpp IsAntineutrino.cpp IsNeutrino.cpp MuonMomentum.cpp Q3Range.cpp TrackAngle.cpp)

install(FILES Cut.h Helicity.h MuonMomentum.h Q3Range.h TrackAngle.h DESTINATION include)
