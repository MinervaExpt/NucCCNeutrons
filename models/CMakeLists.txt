add_library(model OBJECT Model.cpp WithNoArgs.cpp)
install(FILES Model.h WithNoArgs.h DESTINATION include)
