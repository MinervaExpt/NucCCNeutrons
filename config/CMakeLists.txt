file(GLOB *.yaml configs)

install(FILES ${configs} DESTINATION include/config)
