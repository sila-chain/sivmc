# SIVMC: Sila VM Connector API.
# Copyright 2019 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

add_library(mocked_host INTERFACE)
target_sources(mocked_host INTERFACE $<BUILD_INTERFACE:${SIVMC_INCLUDE_DIR}/sivmc/mocked_host.hpp>)

add_library(sivmc::mocked_host ALIAS mocked_host)
target_link_libraries(mocked_host INTERFACE sivmc::sivmc_cpp)

if(SIVMC_INSTALL)
    install(TARGETS mocked_host EXPORT sivmcTargets)
endif()
