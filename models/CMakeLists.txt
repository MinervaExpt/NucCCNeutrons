add_library(models OBJECT Model.cpp WithNoArgs.cpp Flux.cpp)
install(FILES Model.h WithNoArgs.h Flux.h DESTINATION include)
