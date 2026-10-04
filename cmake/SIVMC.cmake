# SIVMC: Sila VM Connector API.
# Copyright 2019 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.


# Adds a CMake test to check the given SIVMC VM implementation with the sivmc-vmtester tool.
#
# sivmc_add_vm_test(NAME <test_name> TARGET <vm>)
# - NAME argument specifies the name of the added test,
# - TARGET argument specifies the CMake target being a shared library with SIVMC VM implementation.
function(sivmc_add_vm_test)
    if(NOT TARGET sivmc::sivmc-vmtester)
        message(FATAL_ERROR "The sivmc-vmtester has not been installed with this SIVMC package")
    endif()

    cmake_parse_arguments("" "" NAME;TARGET "" ${ARGN})
    add_test(NAME ${_NAME} COMMAND sivmc::sivmc-vmtester $<TARGET_FILE:${_TARGET}>)
endfunction()
