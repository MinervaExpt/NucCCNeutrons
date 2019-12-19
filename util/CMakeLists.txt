add_library(support SafeROOTName.cpp Categorized.cpp Directory.cpp)
target_link_libraries(support ${ROOT_LIBRARIES})
install(TARGETS support DESTINATION lib)
install(FILES SafeROOTName.h Categorized.cpp Directory.h DESTINATION include)
