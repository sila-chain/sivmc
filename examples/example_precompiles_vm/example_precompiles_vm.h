// SIVMC: Sila VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <sivmc/sivmc.h>
#include <sivmc/utils.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates SIVMC Example Precompiles VM.
 */
SIVMC_EXPORT struct sivmc_vm* sivmc_create_example_precompiles_vm(void);

#ifdef __cplusplus
}
#endif
