add_library(signal Signal.cpp)
target_link_libraries(signal util)
install(TARGETS signal DESTINATION lib)
install(FILES CrossSection.h Signal.h DESTINATION include/signal)
