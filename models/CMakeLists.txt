add_library(models OBJECT Model.cpp WithNoArgs.cpp Flux.cpp NonResPiSuppression.cpp)
install(FILES Model.h WithNoArgs.h Flux.h NonResPiSuppression.h DESTINATION include)
