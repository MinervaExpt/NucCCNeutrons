#Set up a component library to force the plugin-loading code to detect these files when main() is built.
add_library(systematics OBJECT DropGENIENeutrons.cpp DropGEANTNeutrons.cpp)
