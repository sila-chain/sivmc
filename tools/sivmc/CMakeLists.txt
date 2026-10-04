# SIVMC: Sila VM Connector API.
# Copyright 2019-2020 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

hunter_add_package(CLI11)
find_package(CLI11 REQUIRED)

add_executable(sivmc-tool main.cpp)
add_executable(sivmc::tool ALIAS sivmc-tool)
set_target_properties(sivmc-tool PROPERTIES OUTPUT_NAME sivmc)
set_source_files_properties(main.cpp PROPERTIES
    COMPILE_DEFINITIONS PROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(sivmc-tool PRIVATE sivmc::tooling sivmc::loader CLI11::CLI11)
