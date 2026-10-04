// SIVMC: Sila VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

// Test compilation of C and C++ public headers.

#include <sivmc/sivmc.h>
#include <sivmc/sivmc.hpp>
#include <sivmc/filter_iterator.hpp>
#include <sivmc/helpers.h>
#include <sivmc/hex.hpp>
#include <sivmc/instructions.h>
#include <sivmc/loader.h>
#include <sivmc/mocked_host.hpp>
#include <sivmc/utils.h>

// Include again to check if headers have proper include guards.
#include <sivmc/sivmc.h>               //NOLINT(readability-duplicate-include)
#include <sivmc/sivmc.hpp>             //NOLINT(readability-duplicate-include)
#include <sivmc/filter_iterator.hpp>  //NOLINT(readability-duplicate-include)
#include <sivmc/helpers.h>            //NOLINT(readability-duplicate-include)
#include <sivmc/hex.hpp>              //NOLINT(readability-duplicate-include)
#include <sivmc/instructions.h>       //NOLINT(readability-duplicate-include)
#include <sivmc/loader.h>             //NOLINT(readability-duplicate-include)
#include <sivmc/mocked_host.hpp>      //NOLINT(readability-duplicate-include)
#include <sivmc/utils.h>              //NOLINT(readability-duplicate-include)
