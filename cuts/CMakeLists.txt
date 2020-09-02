add_subdirectory(truth)
add_subdirectory(reco)

add_library(cutter Cutter.cpp)
target_link_libraries(cutter support)
install(TARGETS cutter DESTINATION lib)
