add_library(signal Signal.cpp NeutronDetection.cpp)
target_link_libraries(signal util)
install(TARGETS signal DESTINATION lib)
install(FILES CrossSection.h Signal.h NeutronDetection.h DESTINATION include/signal)
