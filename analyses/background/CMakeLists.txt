add_library(background Background.cpp)
target_link_libraries(background util)
install(TARGETS background DESTINATION lib)
install(FILES CrossSection.h DESTINATION include/background)
