add_library(evt CVUniverse.cpp)
target_link_libraries(evt PlotUtils ROOT)
install(TARGETS evt DESTINATION lib)
install(FILES CVUniverse.h DESTINATION include)
