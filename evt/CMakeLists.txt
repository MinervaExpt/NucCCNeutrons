add_library(evt CVUniverse.cpp)
target_link_libraries(evt PlotUtils ${ROOT_LIBRARIES})
install(TARGETS evt DESTINATION lib)
install(FILES CVUniverse.h DESTINATION include)
