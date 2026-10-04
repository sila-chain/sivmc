// EVMC: Ethereum Client-VM Connector API
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include "vmtester.hpp"
#include <evmc/evmc.hpp>
#include <evmc/mocked_host.hpp>
#include <array>
#include <cstring>

namespace
{
// NOTE: this is to avoid compiler optimisations when reading the buffer
uint8_t read_uint8(const volatile uint8_t* p) noexcept
{
    return *p;
}

void read_buffer(const uint8_t* ptr, size_t size) noexcept
{
    for (size_t i = 0; i < size; i++)
        read_uint8(&ptr[i]);
}
}  // namespace

TEST_F(evmc_vm_test, abi_version_match)
{
    ASSERT_EQ(vm->abi_version, EVMC_ABI_VERSION);
}

TEST_F(evmc_vm_test, name)
{
    ASSERT_TRUE(vm->name != nullptr);
    EXPECT_NE(std::strlen(vm->name), size_t{0}) << "VM name cannot be empty";

    EXPECT_STREQ(owned_vm.name(), vm->name);
}

TEST_F(evmc_vm_test, version)
{
    ASSERT_TRUE(vm->version != nullptr);
    EXPECT_NE(std::strlen(vm->version), size_t{0}) << "VM version cannot be empty";

    EXPECT_STREQ(owned_vm.version(), vm->version);
}

TEST_F(evmc_vm_test, execute_call)
{
    evmc::MockedHost mockedHost;
    const evmc_message msg{};
    std::array<uint8_t, 2> code = {{0xfe, 0x00}};

    const evmc_result result =
        vm->execute(vm, &evmc::MockedHost::get_interface(), mockedHost.to_context(),
                    EVMC_MAX_REVISION, &msg, code.data(), code.size());

    // Validate some constraints
    if (result.status_code != EVMC_SUCCESS && result.status_code != EVMC_REVERT)
    {
        EXPECT_EQ(result.gas_left, 0);
    }

    if (result.output_data == nullptr)
    {
        EXPECT_EQ(result.output_size, size_t{0});
    }
    else
    {
        EXPECT_NE(result.output_size, size_t{0});
        read_buffer(result.output_data, result.output_size);
    }

    if (result.release != nullptr)
        result.release(&result);
}

TEST_F(evmc_vm_test, execute_create)
{
    evmc::MockedHost mockedHost;
    const evmc_message msg{EVMC_CREATE,
                           0,
                           0,
                           65536,
                           0,
                           evmc_address{},
                           evmc_address{},
                           nullptr,
                           0,
                           evmc_uint256be{},
                           evmc_address{},
                           nullptr,
                           0};
    std::array<uint8_t, 2> code = {{0xfe, 0x00}};

    const evmc_result result =
        vm->execute(vm, &evmc::MockedHost::get_interface(), mockedHost.to_context(),
                    EVMC_MAX_REVISION, &msg, code.data(), code.size());

    // Validate some constraints
    if (result.status_code != EVMC_SUCCESS && result.status_code != EVMC_REVERT)
    {
        EXPECT_EQ(result.gas_left, 0);
    }

    if (result.output_data == nullptr)
    {
        EXPECT_EQ(result.output_size, size_t{0});
    }
    else
    {
        EXPECT_NE(result.output_size, size_t{0});
        read_buffer(result.output_data, result.output_size);
    }

    // The VM will never provide the create address.
    if (result.release != nullptr)
        result.release(&result);
}

TEST_F(evmc_vm_test, set_option_unknown_name)
{
    if (vm->set_option != nullptr)
    {
        evmc_set_option_result r = vm->set_option(vm, "unknown_option_csk9twq", "v");
        EXPECT_EQ(r, EVMC_SET_OPTION_INVALID_NAME);
        r = vm->set_option(vm, "unknown_option_csk9twq", "x");
        EXPECT_EQ(r, EVMC_SET_OPTION_INVALID_NAME);
    }
}

TEST_F(evmc_vm_test, set_option_empty_value)
{
    if (vm->set_option != nullptr)
    {
        const auto r = vm->set_option(vm, "unknown_option_csk9twq", nullptr);
        EXPECT_EQ(r, EVMC_SET_OPTION_INVALID_NAME);
    }
}

TEST_F(evmc_vm_test, set_option_unknown_value)
{
    auto r = evmc_set_option(vm, "verbose", "1");

    // Execute more tests if the VM supports "verbose" option.
    if (r != EVMC_SET_OPTION_INVALID_NAME)
    {
        // The VM supports "verbose" option. Try dummy value for it.
        auto r2 = evmc_set_option(vm, "verbose", "GjNOONsbUl");
        EXPECT_EQ(r2, EVMC_SET_OPTION_INVALID_VALUE);

        // For null the behavior should be the same.
        auto r3 = evmc_set_option(vm, "verbose", nullptr);
        EXPECT_EQ(r3, EVMC_SET_OPTION_INVALID_VALUE);
    }
}
