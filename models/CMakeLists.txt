add_library(models OBJECT Model.cpp WithNoArgs.cpp Flux.cpp GENIEUncertainties.cpp MINOSEfficiency.cpp)
install(FILES Model.h WithNoArgs.h Flux.h GENIEUncertainties.h DESTINATION include)
