// SIVMC: Sila VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

/** This example shows how to use sivmc INTERFACE library from sivmc CMake package. */

#include <sivmc/sivmc.h>

int main()
{
    struct sivmc_vm vm = {.abi_version = SIVMC_ABI_VERSION};
    return vm.abi_version - SIVMC_ABI_VERSION;
}
