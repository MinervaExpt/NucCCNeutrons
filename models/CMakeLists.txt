add_library(model OBJECT Model.cpp WithNoArgs.cpp Flux.cpp)
install(FILES Model.h WithNoArgs.h Flux.h DESTINATION include)
