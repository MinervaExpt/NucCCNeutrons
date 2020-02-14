add_library(signal OBJECT Signal.cpp NeutronDetection.cpp)
install(FILES CrossSection.h Signal.h NeutronDetection.h DESTINATION include/signal)
