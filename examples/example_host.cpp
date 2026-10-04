// SIVMC: Sila VM Connector API.
// Copyright 2016 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

/// @file
/// Example implementation of an SIVMC Host.

#include "example_host.h"

#include <sivmc/sivmc.hpp>

#include <algorithm>
#include <map>
#include <vector>

using namespace sivmc::literals;

namespace sivmc
{
struct account
{
    virtual ~account() = default;

    sivmc::uint256be balance = {};
    uint64_t nonce = 0;
    std::vector<uint8_t> code;
    std::map<sivmc::bytes32, sivmc::bytes32> storage;
    std::map<sivmc::bytes32, sivmc::bytes32> transient_storage;

    virtual sivmc::bytes32 code_hash() const
    {
        // Extremely dumb "hash" function.
        sivmc::bytes32 ret{};
        for (const auto v : code)
            ret.bytes[v % sizeof(ret.bytes)] ^= v;
        return ret;
    }
};

using accounts = std::map<sivmc::address, account>;

}  // namespace sivmc

class ExampleHost : public sivmc::Host
{
    sivmc::accounts accounts;
    sivmc_tx_context tx_context{};

public:
    ExampleHost() = default;
    explicit ExampleHost(sivmc_tx_context& _tx_context) noexcept : tx_context{_tx_context} {}
    ExampleHost(sivmc_tx_context& _tx_context, sivmc::accounts& _accounts) noexcept
      : accounts{_accounts}, tx_context{_tx_context}
    {}

    bool account_exists(const sivmc::address& addr) const noexcept final
    {
        return accounts.find(addr) != accounts.end();
    }

    sivmc::bytes32 get_storage(const sivmc::address& addr,
                               const sivmc::bytes32& key) const noexcept final
    {
        const auto account_iter = accounts.find(addr);
        if (account_iter == accounts.end())
            return {};

        const auto storage_iter = account_iter->second.storage.find(key);
        if (storage_iter != account_iter->second.storage.end())
            return storage_iter->second;
        return {};
    }

    sivmc_storage_status set_storage(const sivmc::address& addr,
                                     const sivmc::bytes32& key,
                                     const sivmc::bytes32& value) noexcept final
    {
        auto& account = accounts[addr];
        auto prev_value = account.storage[key];
        account.storage[key] = value;

        return (prev_value == value) ? SIVMC_STORAGE_ASSIGNED : SIVMC_STORAGE_MODIFIED;
    }

    sivmc::uint256be get_balance(const sivmc::address& addr) const noexcept final
    {
        auto it = accounts.find(addr);
        if (it != accounts.end())
            return it->second.balance;
        return {};
    }

    uint64_t get_nonce(const sivmc::address& addr) const noexcept final
    {
        auto it = accounts.find(addr);
        if (it != accounts.end())
            return it->second.nonce;
        return 0;
    }

    size_t get_code_size(const sivmc::address& addr) const noexcept final
    {
        auto it = accounts.find(addr);
        if (it != accounts.end())
            return it->second.code.size();
        return 0;
    }

    sivmc::bytes32 get_code_hash(const sivmc::address& addr) const noexcept final
    {
        auto it = accounts.find(addr);
        if (it != accounts.end())
            return it->second.code_hash();
        return {};
    }

    size_t copy_code(const sivmc::address& addr,
                     size_t code_offset,
                     uint8_t* buffer_data,
                     size_t buffer_size) const noexcept final
    {
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return 0;

        const auto& code = it->second.code;

        if (code_offset >= code.size())
            return 0;

        const auto n = std::min(buffer_size, code.size() - code_offset);

        if (n > 0)
            std::copy_n(&code[code_offset], n, buffer_data);
        return n;
    }

    bool selfdestruct(const sivmc::address& addr, const sivmc::address& beneficiary) noexcept final
    {
        (void)addr;
        (void)beneficiary;
        return false;
    }

    sivmc::Result call(const sivmc_message& msg) noexcept final
    {
        return sivmc::Result{SIVMC_REVERT, msg.gas, 0, msg.input_data, msg.input_size};
    }

    sivmc_tx_context get_tx_context() const noexcept final { return tx_context; }

    // NOLINTNEXTLINE(bugprone-exception-escape)
    sivmc::bytes32 get_block_hash(int64_t number) const noexcept final
    {
        const int64_t current_block_number = get_tx_context().block_number;

        return (number < current_block_number && number >= current_block_number - 256) ?
                   0xb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5f_bytes32 :
                   0x0000000000000000000000000000000000000000000000000000000000000000_bytes32;
    }

    void emit_log(const sivmc::address& addr,
                  const uint8_t* data,
                  size_t data_size,
                  const sivmc::bytes32 topics[],
                  size_t topics_count) noexcept final
    {
        (void)addr;
        (void)data;
        (void)data_size;
        (void)topics;
        (void)topics_count;
    }

    sivmc_access_status access_account(const sivmc::address& addr) noexcept final
    {
        (void)addr;
        return SIVMC_ACCESS_COLD;
    }

    sivmc_access_status access_storage(const sivmc::address& addr,
                                       const sivmc::bytes32& key) noexcept final
    {
        (void)addr;
        (void)key;
        return SIVMC_ACCESS_COLD;
    }

    sivmc::bytes32 get_transient_storage(const sivmc::address& addr,
                                         const sivmc::bytes32& key) const noexcept override
    {
        const auto account_iter = accounts.find(addr);
        if (account_iter == accounts.end())
            return {};

        const auto transient_storage_iter = account_iter->second.transient_storage.find(key);
        if (transient_storage_iter != account_iter->second.transient_storage.end())
            return transient_storage_iter->second;
        return {};
    }

    void set_transient_storage(const sivmc::address& addr,
                               const sivmc::bytes32& key,
                               const sivmc::bytes32& value) noexcept override
    {
        accounts[addr].transient_storage[key] = value;
    }
};


extern "C" {

const sivmc_host_interface* example_host_get_interface()
{
    return &sivmc::Host::get_interface();
}

sivmc_host_context* example_host_create_context(sivmc_tx_context tx_context)
{
    auto host = new ExampleHost{tx_context};
    return host->to_context();
}

void example_host_destroy_context(sivmc_host_context* context)
{
    delete sivmc::Host::from_context<ExampleHost>(context);
}
}
