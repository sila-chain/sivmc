# SIVMC: Sila VM Connector API.
# Copyright 2018 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

# This example shows how to use sivmc INTERFACE library from sivmc CMake package.

cmake_minimum_required(VERSION 3.5)

project(use_sivmc_in_cmake)

find_package(sivmc CONFIG REQUIRED)

add_executable(use_sivmc_in_cmake use_sivmc_in_cmake.c)
target_link_libraries(use_sivmc_in_cmake PRIVATE sivmc::sivmc)



# Only for integration tests.
if(NOT COMMAND sivmc_add_vm_test)
    message(FATAL_ERROR "Function sivmc_add_vm_test() not in SIVMC.cmake module")
endif()
