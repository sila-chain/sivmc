# SIVMC: Sila VM Connector API.
# Copyright 2018 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

add_library(
    instructions STATIC
    ${SIVMC_INCLUDE_DIR}/sivmc/instructions.h
    instruction_metrics.c
    instruction_names.c
)

add_library(sivmc::instructions ALIAS instructions)
set_target_properties(instructions PROPERTIES
    OUTPUT_NAME sivmc-instructions
    POSITION_INDEPENDENT_CODE TRUE
)
target_include_directories(instructions PUBLIC
    $<BUILD_INTERFACE:${SIVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>
)

if(SIVMC_INSTALL)
    install(TARGETS instructions EXPORT sivmcTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
