// SIVMC: Sila VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include <sivmc/helpers.h>

#include <gtest/gtest.h>

// Compile time checks:

static_assert(sizeof(sivmc_bytes32) == 32, "sivmc_bytes32 is too big");
static_assert(sizeof(sivmc_address) == 20, "sivmc_address is too big");
static_assert(sizeof(sivmc_vm) <= 64, "sivmc_vm does not fit cache line");
static_assert(offsetof(sivmc_message, value) % sizeof(size_t) == 0,
              "sivmc_message.value not aligned");

// Check enums match int size.
// On GCC/clang the underlying type should be unsigned int, on MSVC int
static_assert(sizeof(sivmc_call_kind) == sizeof(int),
              "Enum `sivmc_call_kind` is not the size of int");
static_assert(sizeof(sivmc_revision) == sizeof(int),
              "Enum `sivmc_revision` is not the size of int");

TEST(helpers, release_result)
{
    auto r1 = sivmc_result{};
    sivmc_release_result(&r1);

    static sivmc_result r2;
    static bool e;

    e = false;
    r2 = sivmc_result{};
    r2.release = [](const sivmc_result* r) { e = r == &r2; };
    EXPECT_FALSE(e);
    sivmc_release_result(&r2);
    EXPECT_TRUE(e);
}
