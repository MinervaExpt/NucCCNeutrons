add_library(app CmdLine.cpp IsMC.cpp GetPlaylist.cpp SetupPlugins.cpp)
target_link_libraries(app ${ROOT_LIBRARIES} yaml-cpp PlotUtils analysesBase)
install(TARGETS app DESTINATION lib)
install(FILES CmdLine.h IsMC.h GetPlaylist.h SetupPlugins.h DESTINATION include)
