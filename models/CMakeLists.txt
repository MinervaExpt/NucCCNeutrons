add_library(models OBJECT Model.cpp WithNoArgs.cpp Flux.cpp GENIEUncertainties.cpp DansPtRecoilWeight.cpp)
install(FILES Model.h WithNoArgs.h Flux.h GENIEUncertainties.h DansPtRecoilWeight.h DESTINATION include)
