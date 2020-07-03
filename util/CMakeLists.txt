add_library(support SafeROOTName.cpp Directory.cpp StreamRedirection.cpp CaloCorrection.cpp)
target_link_libraries(support ${ROOT_LIBRARIES})
install(TARGETS support DESTINATION lib)
install(FILES SafeROOTName.h Categorized.h Directory.h WithUnits.h units.h Table.h DESTINATION include)
