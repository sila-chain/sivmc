// SIVMC: Sila VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

/** This example shows how to use sivmc::instructions library from sivmc CMake package. */

#include <sivmc/instructions.h>

int main()
{
    return sivmc_get_instruction_metrics_table(SIVMC_BYZANTIUM)[OP_STOP].gas_cost;
}
