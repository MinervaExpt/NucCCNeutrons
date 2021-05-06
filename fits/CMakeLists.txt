add_library(backgroundFits OBJECT ScaleFactor.cpp LinearFit.cpp)

add_library(fits Background.cpp Universe.cpp Sideband.cpp)
target_link_libraries(fits util ${ROOT_LIBRARIES} PlotUtils)
install(TARGETS fits DESTINATION lib)
