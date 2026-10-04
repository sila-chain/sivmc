# SIVMC: Sila VM Connector API.
# Copyright 2018 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

add_library(sivmc INTERFACE)
add_library(sivmc::sivmc ALIAS sivmc)
target_compile_features(sivmc INTERFACE c_std_99)
target_include_directories(sivmc INTERFACE $<BUILD_INTERFACE:${SIVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>)

add_library(sivmc_cpp INTERFACE)
add_library(sivmc::sivmc_cpp ALIAS sivmc_cpp)
target_compile_features(sivmc_cpp INTERFACE cxx_std_17)
target_include_directories(sivmc_cpp INTERFACE $<BUILD_INTERFACE:${SIVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>)
target_link_libraries(sivmc_cpp INTERFACE sivmc::sivmc)

add_subdirectory(instructions)
add_subdirectory(loader)
add_subdirectory(mocked_host)
add_subdirectory(tooling)

if(SIVMC_INSTALL)
    install(TARGETS sivmc sivmc_cpp EXPORT sivmcTargets)
endif()
