add_library(evt Universe.cpp EventID.cpp arachne.cpp)
target_link_libraries(evt PlotUtils ${ROOT_LIBRARIES})
install(TARGETS evt DESTINATION lib)
install(FILES Universe.h EventID.h arachne.h WeightCachedUniverse.h DESTINATION include)
