add_library(background Background.cpp)
target_link_libraries(background util)
install(TARGETS background DESTINATION lib)
install(FILES Background.h DESTINATION include)

add_subdirectory(signal)
add_subdirectory(sideband)
#add_subdirectory(multiplicity)

add_library(analyses OBJECT MuonMomentum.cpp)
