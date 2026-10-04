// SIVMC: Sila VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include "example_precompiles_vm.h"
#include <algorithm>

namespace
{
sivmc_result execute_identity(const sivmc_message* msg)
{
    auto result = sivmc_result{};

    // Check the gas cost.
    auto gas_cost = 15 + 3 * ((int64_t(msg->input_size) + 31) / 32);
    auto gas_left = msg->gas - gas_cost;
    if (gas_left < 0)
    {
        result.status_code = SIVMC_OUT_OF_GAS;
        return result;
    }

    // Execute.
    auto data = new uint8_t[msg->input_size];
    std::copy_n(msg->input_data, msg->input_size, data);

    // Return the result.
    result.status_code = SIVMC_SUCCESS;
    result.output_data = data;
    result.output_size = msg->input_size;
    result.release = [](const sivmc_result* r) { delete[] r->output_data; };
    result.gas_left = gas_left;
    return result;
}

sivmc_result execute_empty(const sivmc_message* msg)
{
    auto result = sivmc_result{};
    result.status_code = SIVMC_SUCCESS;
    result.gas_left = msg->gas;
    return result;
}

sivmc_result not_implemented()
{
    auto result = sivmc_result{};
    result.status_code = SIVMC_REJECTED;
    return result;
}

sivmc_result execute(sivmc_vm* /*vm*/,
                     const sivmc_host_interface* /*host*/,
                     sivmc_host_context* /*context*/,
                     enum sivmc_revision rev,
                     const sivmc_message* msg,
                     const uint8_t* /*code*/,
                     size_t /*code_size*/)
{
    // The SIP-1352 (https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-1352.md) defines
    // the range 0 - 0xffff (2 bytes) of addresses reserved for precompiled contracts.
    // Check if the code address is within the reserved range.

    constexpr auto prefix_size = sizeof(sivmc_address) - 2;
    const auto& addr = msg->code_address;
    // Check if the address prefix is all zeros.
    if (std::any_of(&addr.bytes[0], &addr.bytes[prefix_size], [](uint8_t x) { return x != 0; }))
    {
        // If not, reject the execution request.
        auto result = sivmc_result{};
        result.status_code = SIVMC_REJECTED;
        return result;
    }

    // Extract the precompiled contract id from last 2 bytes of the code address.
    const auto id = (addr.bytes[prefix_size] << 8) | addr.bytes[prefix_size + 1];
    switch (id)
    {
    case 0x0001:  // ECDSARECOVER
    case 0x0002:  // SHA256
    case 0x0003:  // RIPEMD160
        return not_implemented();

    case 0x0004:  // Identity
        return execute_identity(msg);

    case 0x0005:  // EXPMOD
    case 0x0006:  // SNARKV
    case 0x0007:  // BNADD
    case 0x0008:  // BNMUL
        if (rev < SIVMC_SILA_BYZANTIUM)
            return execute_empty(msg);
        return not_implemented();

    default:  // As if empty code was executed.
        return execute_empty(msg);
    }
}
}  // namespace

sivmc_vm* sivmc_create_example_precompiles_vm()
{
    static struct sivmc_vm vm = {
        SIVMC_ABI_VERSION, "example_precompiles_vm", PROJECT_VERSION, [](sivmc_vm*) {}, execute,
        nullptr,
    };
    return &vm;
}
