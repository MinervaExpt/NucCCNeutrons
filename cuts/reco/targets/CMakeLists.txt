#Set up a component library to force the plugin-loading code to detect these files when main() is built.
add_library(recoTargets OBJECT Target1Iron.cpp Target1Lead.cpp Target2Iron.cpp Target2Lead.cpp Target3Carbon.cpp Target3Iron.cpp Target3Lead.cpp
                               Target5Iron.cpp Target5Lead.cpp IsInTarget.cpp Between.cpp OneSectionTarget.cpp)

install(FILES OneSectionTarget.h TwoSectionTarget.h ThreeSectionTarget.h IsInTarget.h Between.h DESTINATION include)
