add_subdirectory(targets)

#Set up a component library to force the plugin-loading code to detect these files when main() is built.
add_library(recoCuts OBJECT Cut.cpp IsAntineutrino.cpp IsNeutrino.cpp MuonMomentum.cpp Q3Range.cpp TrackAngle.cpp MinosDeltaT.cpp nTracks.cpp Apothem.cpp RecoilERange.cpp HasInteractionVertex.cpp DeadDiscriminators.cpp)

install(FILES Cut.h Helicity.h MuonMomentum.h Q3Range.h TrackAngle.h nTracks.h MinosDeltaT.h Apothem.h RecoilERange.h HasInteractionVertex.h DeadDiscriminators.h DESTINATION include)
