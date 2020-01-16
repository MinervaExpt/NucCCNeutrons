add_library(sideband Sideband.cpp)
target_link_libraries(sideband util)
install(TARGETS sideband DESTINATION lib)
install(FILES CrossSection.h DESTINATION include/sideband)
