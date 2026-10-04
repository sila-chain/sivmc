# SIVMC: Sila VM Connector API.
# Copyright 2021 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

add_library(tooling STATIC)
add_library(sivmc::tooling ALIAS tooling)
target_compile_features(tooling PUBLIC cxx_std_17)
target_link_libraries(tooling PUBLIC sivmc::sivmc_cpp sivmc::mocked_host)

target_sources(
    tooling PRIVATE
    ${SIVMC_INCLUDE_DIR}/sivmc/tooling.hpp
    run.cpp
)

if(SIVMC_INSTALL)
    install(TARGETS tooling EXPORT sivmcTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
