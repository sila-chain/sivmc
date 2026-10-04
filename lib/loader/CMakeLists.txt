# SIVMC: Sila VM Connector API.
# Copyright 2018 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

add_library(
    loader STATIC
    ${SIVMC_INCLUDE_DIR}/sivmc/loader.h
    loader.c
)

add_library(sivmc::loader ALIAS loader)
set_target_properties(loader PROPERTIES
    OUTPUT_NAME sivmc-loader
    POSITION_INDEPENDENT_CODE TRUE
)
target_link_libraries(loader INTERFACE ${CMAKE_DL_LIBS} PUBLIC sivmc::sivmc)

if(SIVMC_INSTALL)
    install(TARGETS loader EXPORT sivmcTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
