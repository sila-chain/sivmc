# SIVMC: Sila VM Connector API.
# Copyright 2016 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

include(GNUInstallDirs)

if(MSVC AND CMAKE_C_COMPILER_VERSION VERSION_LESS 19.20)
    # For Visual Studio _before_ 2019
    # allow non-constant aggregate initializer.
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C>:/wd4204>
        $<$<COMPILE_LANGUAGE:C>:/wd4221>
    )
endif()

add_subdirectory(example_vm)
add_subdirectory(example_precompiles_vm)

add_library(sivmc-example-host STATIC example_host.cpp)
target_link_libraries(sivmc-example-host PRIVATE sivmc::sivmc_cpp)

add_executable(sivmc-example-static example.c)
target_compile_features(sivmc-example-static PRIVATE c_std_99)
target_link_libraries(sivmc-example-static PRIVATE sivmc-example-host sivmc::example-vm-static sivmc::sivmc)
target_compile_definitions(sivmc-example-static PRIVATE STATICALLY_LINKED_EXAMPLE)

add_executable(sivmc-example example.c)
target_compile_features(sivmc-example PRIVATE c_std_99)
target_link_libraries(sivmc-example PRIVATE sivmc-example-host sivmc::loader)
