add_library(studies OBJECT MuonMomentum.cpp NeutronMultiplicity.cpp NeutronDetection.cpp)
install(FILES CrossSectionSignal.h CrossSectionSideband.h NeutronDetection.h DESTINATION include)
