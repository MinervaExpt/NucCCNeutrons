#Set up a component library to force the plugin-loading code to detect these files when main() is built.
add_library(truthTargets OBJECT Target1Iron.cpp Target1Lead.cpp Target2Iron.cpp Target2Lead.cpp Target3Carbon.cpp Target3Iron.cpp Target3Lead.cpp
                                Water.cpp Target4Carbon.cpp Target5Iron.cpp Target5Lead.cpp IsInTarget.cpp)

install(FILES OneSectionTarget.h TwoSectionTarget.h ThreeSectionTarget.h IsInTarget.h DESTINATION include)
