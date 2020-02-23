add_library(studies OBJECT MuonMomentum.cpp NeutronMultiplicity.cpp NeutronDetection.cpp EAvailable.cpp)
install(FILES CrossSectionSignal.h CrossSectionSideband.h NeutronDetection.h Resolution.h DESTINATION include)
