add_library(analysesBase Study.cpp Background.cpp)
target_link_libraries(analysesBase util)
install(TARGETS analysesBase DESTINATION lib)
install(FILES Background.h Study.h DESTINATION include)
