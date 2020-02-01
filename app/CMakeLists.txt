add_library(app CmdLine.cpp IsMC.cpp GetPlaylist.cpp)
target_link_libraries(app ${ROOT_LIBRARIES} yaml-cpp PlotUtils) #PlotUtils_dict)
install(TARGETS app DESTINATION lib)
install(FILES CmdLine.h IsMC.h GetPlaylist.h DESTINATION include)
