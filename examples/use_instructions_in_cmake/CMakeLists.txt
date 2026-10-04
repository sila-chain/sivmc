# SIVMC: Sila VM Connector API.
# Copyright 2018 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

# This example shows how to use sivmc::instructions library from sivmc CMake package.

cmake_minimum_required(VERSION 3.5)

project(use_instructions_in_cmake)

find_package(sivmc CONFIG REQUIRED)

add_executable(use_instructions_in_cmake use_instructions_in_cmake.c)
target_link_libraries(use_instructions_in_cmake PRIVATE sivmc::instructions)
