add_library(evt Universe.cpp EventID.cpp arachne.cpp)
target_link_libraries(evt MAT MAT-MINERvA ${ROOT_LIBRARIES})
install(TARGETS evt DESTINATION lib)
install(FILES Universe.h EventID.h arachne.h DESTINATION include)
