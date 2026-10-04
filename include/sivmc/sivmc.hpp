// SIVMC: Sila VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <sivmc/helpers.h>
#include <sivmc/hex.hpp>
#include <sivmc/sivmc.h>

#include <functional>
#include <initializer_list>
#include <ostream>
#include <string_view>
#include <utility>

static_assert(SIVMC_LATEST_STABLE_REVISION <= SIVMC_MAX_REVISION,
              "latest stable revision ill-defined");

/// SIVMC C++ API - wrappers and bindings for C++
/// @ingroup cpp
namespace sivmc
{
/// The big-endian 160-bit hash suitable for keeping a Sila address.
///
/// This type wraps C ::sivmc_address to make sure objects of this type are always initialized.
struct address : sivmc_address
{
    /// Default and converting constructor.
    ///
    /// Initializes bytes to zeros if not other @p init value provided.
    constexpr address(sivmc_address init = {}) noexcept : sivmc_address{init} {}

    /// Converting constructor from unsigned integer value.
    ///
    /// This constructor assigns the @p v value to the last 8 bytes [12:19]
    /// in big-endian order.
    constexpr explicit address(uint64_t v) noexcept
      : sivmc_address{{0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       static_cast<uint8_t>(v >> 56),
                       static_cast<uint8_t>(v >> 48),
                       static_cast<uint8_t>(v >> 40),
                       static_cast<uint8_t>(v >> 32),
                       static_cast<uint8_t>(v >> 24),
                       static_cast<uint8_t>(v >> 16),
                       static_cast<uint8_t>(v >> 8),
                       static_cast<uint8_t>(v >> 0)}}
    {}

    /// Explicit operator converting to bool.
    inline constexpr explicit operator bool() const noexcept;

    /// Implicit operator converting to bytes_view.
    inline constexpr operator bytes_view() const noexcept { return {bytes, sizeof(bytes)}; }
};

/// The fixed size array of 32 bytes for storing 256-bit Sivm values.
///
/// This type wraps C ::sivmc_bytes32 to make sure objects of this type are always initialized.
struct bytes32 : sivmc_bytes32
{
    /// Default and converting constructor.
    ///
    /// Initializes bytes to zeros if not other @p init value provided.
    constexpr bytes32(sivmc_bytes32 init = {}) noexcept : sivmc_bytes32{init} {}

    /// Converting constructor from unsigned integer value.
    ///
    /// This constructor assigns the @p v value to the last 8 bytes [24:31]
    /// in big-endian order.
    constexpr explicit bytes32(uint64_t v) noexcept
      : sivmc_bytes32{{0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       static_cast<uint8_t>(v >> 56),
                       static_cast<uint8_t>(v >> 48),
                       static_cast<uint8_t>(v >> 40),
                       static_cast<uint8_t>(v >> 32),
                       static_cast<uint8_t>(v >> 24),
                       static_cast<uint8_t>(v >> 16),
                       static_cast<uint8_t>(v >> 8),
                       static_cast<uint8_t>(v >> 0)}}
    {}

    /// Explicit operator converting to bool.
    inline constexpr explicit operator bool() const noexcept;

    /// Implicit operator converting to bytes_view.
    inline constexpr operator bytes_view() const noexcept { return {bytes, sizeof(bytes)}; }
};

/// The alias for sivmc::bytes32 to represent a big-endian 256-bit integer.
using uint256be = bytes32;


/// Loads 64 bits / 8 bytes of data from the given @p data array in big-endian order.
inline constexpr uint64_t load64be(const uint8_t* data) noexcept
{
    return (uint64_t{data[0]} << 56) | (uint64_t{data[1]} << 48) | (uint64_t{data[2]} << 40) |
           (uint64_t{data[3]} << 32) | (uint64_t{data[4]} << 24) | (uint64_t{data[5]} << 16) |
           (uint64_t{data[6]} << 8) | uint64_t{data[7]};
}

/// Loads 64 bits / 8 bytes of data from the given @p data array in little-endian order.
inline constexpr uint64_t load64le(const uint8_t* data) noexcept
{
    return uint64_t{data[0]} | (uint64_t{data[1]} << 8) | (uint64_t{data[2]} << 16) |
           (uint64_t{data[3]} << 24) | (uint64_t{data[4]} << 32) | (uint64_t{data[5]} << 40) |
           (uint64_t{data[6]} << 48) | (uint64_t{data[7]} << 56);
}

/// Loads 32 bits / 4 bytes of data from the given @p data array in big-endian order.
inline constexpr uint32_t load32be(const uint8_t* data) noexcept
{
    return (uint32_t{data[0]} << 24) | (uint32_t{data[1]} << 16) | (uint32_t{data[2]} << 8) |
           uint32_t{data[3]};
}

/// Loads 32 bits / 4 bytes of data from the given @p data array in little-endian order.
inline constexpr uint32_t load32le(const uint8_t* data) noexcept
{
    return uint32_t{data[0]} | (uint32_t{data[1]} << 8) | (uint32_t{data[2]} << 16) |
           (uint32_t{data[3]} << 24);
}

namespace fnv
{
constexpr auto prime = 0x100000001b3;              ///< The 64-bit FNV prime number.
constexpr auto offset_basis = 0xcbf29ce484222325;  ///< The 64-bit FNV offset basis.

/// The hashing transformation for 64-bit inputs based on the FNV-1a formula.
inline constexpr uint64_t fnv1a_by64(uint64_t h, uint64_t x) noexcept
{
    return (h ^ x) * prime;
}
}  // namespace fnv


/// The "equal to" comparison operator for the sivmc::address type.
inline constexpr bool operator==(const address& a, const address& b) noexcept
{
    return load64le(&a.bytes[0]) == load64le(&b.bytes[0]) &&
           load64le(&a.bytes[8]) == load64le(&b.bytes[8]) &&
           load32le(&a.bytes[16]) == load32le(&b.bytes[16]);
}

/// The "not equal to" comparison operator for the sivmc::address type.
inline constexpr bool operator!=(const address& a, const address& b) noexcept
{
    return !(a == b);
}

/// The "less than" comparison operator for the sivmc::address type.
inline constexpr bool operator<(const address& a, const address& b) noexcept
{
    return load64be(&a.bytes[0]) < load64be(&b.bytes[0]) ||
           (load64be(&a.bytes[0]) == load64be(&b.bytes[0]) &&
            (load64be(&a.bytes[8]) < load64be(&b.bytes[8]) ||
             (load64be(&a.bytes[8]) == load64be(&b.bytes[8]) &&
              load32be(&a.bytes[16]) < load32be(&b.bytes[16]))));
}

/// The "greater than" comparison operator for the sivmc::address type.
inline constexpr bool operator>(const address& a, const address& b) noexcept
{
    return b < a;
}

/// The "less than or equal to" comparison operator for the sivmc::address type.
inline constexpr bool operator<=(const address& a, const address& b) noexcept
{
    return !(b < a);
}

/// The "greater than or equal to" comparison operator for the sivmc::address type.
inline constexpr bool operator>=(const address& a, const address& b) noexcept
{
    return !(a < b);
}

/// The "equal to" comparison operator for the sivmc::bytes32 type.
inline constexpr bool operator==(const bytes32& a, const bytes32& b) noexcept
{
    return load64le(&a.bytes[0]) == load64le(&b.bytes[0]) &&
           load64le(&a.bytes[8]) == load64le(&b.bytes[8]) &&
           load64le(&a.bytes[16]) == load64le(&b.bytes[16]) &&
           load64le(&a.bytes[24]) == load64le(&b.bytes[24]);
}

/// The "not equal to" comparison operator for the sivmc::bytes32 type.
inline constexpr bool operator!=(const bytes32& a, const bytes32& b) noexcept
{
    return !(a == b);
}

/// The "less than" comparison operator for the sivmc::bytes32 type.
inline constexpr bool operator<(const bytes32& a, const bytes32& b) noexcept
{
    return load64be(&a.bytes[0]) < load64be(&b.bytes[0]) ||
           (load64be(&a.bytes[0]) == load64be(&b.bytes[0]) &&
            (load64be(&a.bytes[8]) < load64be(&b.bytes[8]) ||
             (load64be(&a.bytes[8]) == load64be(&b.bytes[8]) &&
              (load64be(&a.bytes[16]) < load64be(&b.bytes[16]) ||
               (load64be(&a.bytes[16]) == load64be(&b.bytes[16]) &&
                load64be(&a.bytes[24]) < load64be(&b.bytes[24]))))));
}

/// The "greater than" comparison operator for the sivmc::bytes32 type.
inline constexpr bool operator>(const bytes32& a, const bytes32& b) noexcept
{
    return b < a;
}

/// The "less than or equal to" comparison operator for the sivmc::bytes32 type.
inline constexpr bool operator<=(const bytes32& a, const bytes32& b) noexcept
{
    return !(b < a);
}

/// The "greater than or equal to" comparison operator for the sivmc::bytes32 type.
inline constexpr bool operator>=(const bytes32& a, const bytes32& b) noexcept
{
    return !(a < b);
}

/// Checks if the given address is the zero address.
inline constexpr bool is_zero(const address& a) noexcept
{
    return a == address{};
}

inline constexpr address::operator bool() const noexcept
{
    return !is_zero(*this);
}

/// Checks if the given bytes32 object has all zero bytes.
inline constexpr bool is_zero(const bytes32& a) noexcept
{
    return a == bytes32{};
}

inline constexpr bytes32::operator bool() const noexcept
{
    return !is_zero(*this);
}

namespace literals
{
/// Converts a raw literal into value of type T.
///
/// This function is expected to be used on literals in constexpr context only.
/// In case the input is invalid the std::terminate() is called.
/// TODO(c++20): Use consteval.
template <typename T>
constexpr T parse(std::string_view s) noexcept
{
    return from_hex<T>(s).value();
}

/// Literal for sivmc::address.
constexpr address operator""_address(const char* s) noexcept
{
    return parse<address>(s);
}

/// Literal for sivmc::bytes32.
constexpr bytes32 operator""_bytes32(const char* s) noexcept
{
    return parse<bytes32>(s);
}
}  // namespace literals

using namespace literals;


/// @copydoc sivmc_status_code_to_string
inline const char* to_string(sivmc_status_code status_code) noexcept
{
    return sivmc_status_code_to_string(status_code);
}

/// @copydoc sivmc_revision_to_string
inline const char* to_string(sivmc_revision rev) noexcept
{
    return sivmc_revision_to_string(rev);
}


/// Alias for sivmc_make_result().
constexpr auto make_result = sivmc_make_result;

/// @copydoc sivmc_state_gas
using StateGas = sivmc_state_gas;

/// @copydoc sivmc_result
///
/// This is a RAII wrapper for sivmc_result and objects of this type
/// automatically release attached resources.
class Result : private sivmc_result
{
public:
    using sivmc_result::gas_left;
    using sivmc_result::gas_refund;
    using sivmc_result::output_data;
    using sivmc_result::output_size;
    using sivmc_result::state_gas;
    using sivmc_result::status_code;

    /// Creates the result from the provided arguments.
    ///
    /// The provided output is copied to memory allocated with malloc()
    /// and the sivmc_result::release function is set to one invoking free().
    ///
    /// @param _status_code  The status code.
    /// @param _gas_left     The amount of gas left.
    /// @param _gas_refund   The amount of refunded gas.
    /// @param _output_data  The pointer to the output.
    /// @param _output_size  The output size.
    /// @param _state_gas    The state-gas fields.
    explicit Result(sivmc_status_code _status_code,
                    int64_t _gas_left,
                    int64_t _gas_refund,
                    const uint8_t* _output_data,
                    size_t _output_size,
                    StateGas _state_gas = {}) noexcept
      : sivmc_result{make_result(_status_code, _gas_left, _gas_refund, _output_data, _output_size)}
    {
        state_gas = _state_gas;
    }

    /// Creates the result without output.
    ///
    /// @param _status_code  The status code.
    /// @param _gas_left     The amount of gas left.
    /// @param _gas_refund   The amount of refunded gas.
    /// @param _state_gas    The state-gas fields.
    explicit Result(sivmc_status_code _status_code = SIVMC_INTERNAL_ERROR,
                    int64_t _gas_left = 0,
                    int64_t _gas_refund = 0,
                    StateGas _state_gas = {}) noexcept
      : sivmc_result{make_result(_status_code, _gas_left, _gas_refund, nullptr, 0)}
    {
        state_gas = _state_gas;
    }

    /// Creates the result without output and without gas left.
    ///
    /// @param _status_code  The status code.
    /// @param _state_gas    The state-gas fields.
    explicit Result(sivmc_status_code _status_code, StateGas _state_gas) noexcept
      : Result{_status_code, 0, 0, _state_gas}
    {}

    /// Converting constructor from raw sivmc_result.
    ///
    /// This object takes ownership of the resources of @p res.
    explicit Result(const sivmc_result& res) noexcept : sivmc_result{res} {}

    /// Destructor responsible for automatically releasing attached resources.
    ~Result() noexcept
    {
        if (release != nullptr)
            release(this);
    }

    /// Move constructor.
    Result(Result&& other) noexcept : sivmc_result{other}
    {
        other.release = nullptr;  // Disable releasing of the rvalue object.
    }

    /// Move assignment operator.
    ///
    /// The self-assignment MUST never happen.
    ///
    /// @param other The other result object.
    /// @return      The reference to the left-hand side object.
    Result& operator=(Result&& other) noexcept
    {
        this->~Result();                            // Release this object.
        static_cast<sivmc_result&>(*this) = other;  // Copy data.
        other.release = nullptr;                    // Disable releasing of the rvalue object.
        return *this;
    }

    /// Access the result object as a referenced to ::sivmc_result.
    sivmc_result& raw() noexcept { return *this; }

    /// Access the result object as a const referenced to ::sivmc_result.
    const sivmc_result& raw() const noexcept { return *this; }

    /// Releases the ownership and returns the raw copy of sivmc_result.
    ///
    /// This method drops the ownership of the result
    /// (result's resources are not going to be released when this object is destructed).
    /// It is the caller's responsibility having the returned copy of the result to release it.
    /// This object MUST NOT be used after this method is invoked.
    ///
    /// @return  The copy of this object converted to raw sivmc_result.
    sivmc_result release_raw() noexcept
    {
        const auto out = sivmc_result{*this};  // Copy data.
        this->release = nullptr;               // Disable releasing of this object.
        return out;
    }
};


/// The SIVMC Host interface
class HostInterface
{
public:
    virtual ~HostInterface() noexcept = default;

    /// @copydoc sivmc_host_interface::account_exists
    virtual bool account_exists(const address& addr) const noexcept = 0;

    /// @copydoc sivmc_host_interface::get_storage
    virtual bytes32 get_storage(const address& addr, const bytes32& key) const noexcept = 0;

    /// @copydoc sivmc_host_interface::set_storage
    virtual sivmc_storage_status set_storage(const address& addr,
                                             const bytes32& key,
                                             const bytes32& value) noexcept = 0;

    /// @copydoc sivmc_host_interface::get_balance
    virtual uint256be get_balance(const address& addr) const noexcept = 0;

    /// @copydoc sivmc_host_interface::get_nonce
    virtual uint64_t get_nonce(const address& addr) const noexcept = 0;

    /// @copydoc sivmc_host_interface::get_code_size
    virtual size_t get_code_size(const address& addr) const noexcept = 0;

    /// @copydoc sivmc_host_interface::get_code_hash
    virtual bytes32 get_code_hash(const address& addr) const noexcept = 0;

    /// @copydoc sivmc_host_interface::copy_code
    virtual size_t copy_code(const address& addr,
                             size_t code_offset,
                             uint8_t* buffer_data,
                             size_t buffer_size) const noexcept = 0;

    /// @copydoc sivmc_host_interface::selfdestruct
    virtual bool selfdestruct(const address& addr, const address& beneficiary) noexcept = 0;

    /// @copydoc sivmc_host_interface::call
    virtual Result call(const sivmc_message& msg) noexcept = 0;

    /// @copydoc sivmc_host_interface::get_tx_context
    virtual sivmc_tx_context get_tx_context() const noexcept = 0;

    /// @copydoc sivmc_host_interface::get_block_hash
    virtual bytes32 get_block_hash(int64_t block_number) const noexcept = 0;

    /// @copydoc sivmc_host_interface::emit_log
    virtual void emit_log(const address& addr,
                          const uint8_t* data,
                          size_t data_size,
                          const bytes32 topics[],
                          size_t num_topics) noexcept = 0;

    /// @copydoc sivmc_host_interface::access_account
    virtual sivmc_access_status access_account(const address& addr) noexcept = 0;

    /// @copydoc sivmc_host_interface::access_storage
    virtual sivmc_access_status access_storage(const address& addr,
                                               const bytes32& key) noexcept = 0;

    /// @copydoc sivmc_host_interface::get_transient_storage
    virtual bytes32 get_transient_storage(const address& addr,
                                          const bytes32& key) const noexcept = 0;

    /// @copydoc sivmc_host_interface::set_transient_storage
    virtual void set_transient_storage(const address& addr,
                                       const bytes32& key,
                                       const bytes32& value) noexcept = 0;
};


/// Wrapper around SIVMC host context / host interface.
///
/// To be used by VM implementations as better alternative to using ::sivmc_host_context directly.
class HostContext : public HostInterface
{
    const sivmc_host_interface* host = nullptr;
    sivmc_host_context* context = nullptr;

public:
    /// Default constructor for null Host context.
    HostContext() = default;

    /// Constructor from the SIVMC Host primitives.
    /// @param interface  The reference to the Host interface.
    /// @param ctx        The pointer to the Host context object. This parameter MAY be null.
    HostContext(const sivmc_host_interface& interface, sivmc_host_context* ctx) noexcept
      : host{&interface}, context{ctx}
    {}

    bool account_exists(const address& address) const noexcept final
    {
        return host->account_exists(context, &address);
    }

    bytes32 get_storage(const address& address, const bytes32& key) const noexcept final
    {
        return host->get_storage(context, &address, &key);
    }

    sivmc_storage_status set_storage(const address& address,
                                     const bytes32& key,
                                     const bytes32& value) noexcept final
    {
        return host->set_storage(context, &address, &key, &value);
    }

    uint256be get_balance(const address& address) const noexcept final
    {
        return host->get_balance(context, &address);
    }

    uint64_t get_nonce(const address& address) const noexcept final
    {
        return host->get_nonce(context, &address);
    }

    size_t get_code_size(const address& address) const noexcept final
    {
        return host->get_code_size(context, &address);
    }

    bytes32 get_code_hash(const address& address) const noexcept final
    {
        return host->get_code_hash(context, &address);
    }

    size_t copy_code(const address& address,
                     size_t code_offset,
                     uint8_t* buffer_data,
                     size_t buffer_size) const noexcept final
    {
        return host->copy_code(context, &address, code_offset, buffer_data, buffer_size);
    }

    bool selfdestruct(const address& addr, const address& beneficiary) noexcept final
    {
        return host->selfdestruct(context, &addr, &beneficiary);
    }

    Result call(const sivmc_message& message) noexcept final
    {
        return Result{host->call(context, &message)};
    }

    /// @copydoc HostInterface::get_tx_context()
    sivmc_tx_context get_tx_context() const noexcept final { return host->get_tx_context(context); }

    bytes32 get_block_hash(int64_t number) const noexcept final
    {
        return host->get_block_hash(context, number);
    }

    void emit_log(const address& addr,
                  const uint8_t* data,
                  size_t data_size,
                  const bytes32 topics[],
                  size_t topics_count) noexcept final
    {
        host->emit_log(context, &addr, data, data_size, topics, topics_count);
    }

    sivmc_access_status access_account(const address& address) noexcept final
    {
        return host->access_account(context, &address);
    }

    sivmc_access_status access_storage(const address& address, const bytes32& key) noexcept final
    {
        return host->access_storage(context, &address, &key);
    }

    bytes32 get_transient_storage(const address& address, const bytes32& key) const noexcept final
    {
        return host->get_transient_storage(context, &address, &key);
    }

    void set_transient_storage(const address& address,
                               const bytes32& key,
                               const bytes32& value) noexcept final
    {
        host->set_transient_storage(context, &address, &key, &value);
    }
};


/// Abstract class to be used by Host implementations.
///
/// When implementing SIVMC Host, you can directly inherit from the sivmc::Host class.
/// This way your implementation will be simpler by avoiding manual handling
/// of the ::sivmc_host_context and the ::sivmc_host_interface.
class Host : public HostInterface
{
public:
    /// Provides access to the global host interface.
    /// @returns  Reference to the host interface object.
    static const sivmc_host_interface& get_interface() noexcept;

    /// Converts the Host object to the opaque host context pointer.
    /// @returns  Pointer to sivmc_host_context.
    sivmc_host_context* to_context() noexcept
    {
        return reinterpret_cast<sivmc_host_context*>(this);
    }

    /// Converts the opaque host context pointer back to the original Host object.
    /// @tparam DerivedClass  The class derived from the Host class.
    /// @param context        The opaque host context pointer.
    /// @returns              The pointer to DerivedClass.
    template <typename DerivedClass = Host>
    static DerivedClass* from_context(sivmc_host_context* context) noexcept
    {
        // Get pointer of the Host base class.
        auto* h = reinterpret_cast<Host*>(context);

        // Additional downcast, only possible if DerivedClass inherits from Host.
        return static_cast<DerivedClass*>(h);
    }
};


/// @copybrief sivmc_vm
///
/// This is a RAII wrapper for sivmc_vm, and object of this type
/// automatically destroys the VM instance.
class VM
{
public:
    VM() noexcept = default;

    /// Converting constructor from sivmc_vm.
    explicit VM(sivmc_vm* vm) noexcept : m_instance{vm} {}

    /// Destructor responsible for automatically destroying the VM instance.
    ~VM() noexcept
    {
        if (m_instance != nullptr)
            m_instance->destroy(m_instance);
    }

    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;

    /// Move constructor.
    VM(VM&& other) noexcept : m_instance{other.m_instance} { other.m_instance = nullptr; }

    /// Move assignment operator.
    VM& operator=(VM&& other) noexcept
    {
        this->~VM();
        m_instance = other.m_instance;
        other.m_instance = nullptr;
        return *this;
    }

    /// The constructor that captures a VM instance and configures the instance
    /// with the provided list of options.
    inline VM(sivmc_vm* vm,
              std::initializer_list<std::pair<const char*, const char*>> options) noexcept;

    /// Checks if contains a valid pointer to the VM instance.
    explicit operator bool() const noexcept { return m_instance != nullptr; }

    /// Checks whenever the VM instance is ABI compatible with the current SIVMC API.
    bool is_abi_compatible() const noexcept { return m_instance->abi_version == SIVMC_ABI_VERSION; }

    /// @copydoc sivmc_vm::name
    char const* name() const noexcept { return m_instance->name; }

    /// @copydoc sivmc_vm::version
    char const* version() const noexcept { return m_instance->version; }

    /// @copydoc sivmc_set_option()
    sivmc_set_option_result set_option(const char name[], const char value[]) noexcept
    {
        return sivmc_set_option(m_instance, name, value);
    }

    /// @copydoc sivmc_execute()
    Result execute(const sivmc_host_interface& host,
                   sivmc_host_context* ctx,
                   sivmc_revision rev,
                   const sivmc_message& msg,
                   const uint8_t* code,
                   size_t code_size) noexcept
    {
        return Result{m_instance->execute(m_instance, &host, ctx, rev, &msg, code, code_size)};
    }

    /// Convenient variant of the VM::execute() that takes reference to sivmc::Host class.
    Result execute(Host& host,
                   sivmc_revision rev,
                   const sivmc_message& msg,
                   const uint8_t* code,
                   size_t code_size) noexcept
    {
        return execute(Host::get_interface(), host.to_context(), rev, msg, code, code_size);
    }

    /// Returns the pointer to C SIVMC struct representing the VM.
    ///
    /// Gives access to the C SIVMC VM struct to allow advanced interaction with the VM not
    /// supported by the C++ interface. Use as the last resort. This object still owns the VM after
    /// returning the pointer. The returned pointer MAY be null.
    sivmc_vm* get_raw_pointer() const noexcept { return m_instance; }

private:
    sivmc_vm* m_instance = nullptr;
};

inline VM::VM(sivmc_vm* vm,
              std::initializer_list<std::pair<const char*, const char*>> options) noexcept
  : m_instance{vm}
{
    // This constructor is implemented outside of the class definition to workaround a doxygen bug.
    for (const auto& option : options)
        set_option(option.first, option.second);
}


namespace internal
{
inline bool account_exists(sivmc_host_context* h, const sivmc_address* addr) noexcept
{
    return Host::from_context(h)->account_exists(*addr);
}

inline sivmc_bytes32 get_storage(sivmc_host_context* h,
                                 const sivmc_address* addr,
                                 const sivmc_bytes32* key) noexcept
{
    return Host::from_context(h)->get_storage(*addr, *key);
}

inline sivmc_storage_status set_storage(sivmc_host_context* h,
                                        const sivmc_address* addr,
                                        const sivmc_bytes32* key,
                                        const sivmc_bytes32* value) noexcept
{
    return Host::from_context(h)->set_storage(*addr, *key, *value);
}

inline sivmc_uint256be get_balance(sivmc_host_context* h, const sivmc_address* addr) noexcept
{
    return Host::from_context(h)->get_balance(*addr);
}

inline uint64_t get_nonce(sivmc_host_context* h, const sivmc_address* addr) noexcept
{
    return Host::from_context(h)->get_nonce(*addr);
}

inline size_t get_code_size(sivmc_host_context* h, const sivmc_address* addr) noexcept
{
    return Host::from_context(h)->get_code_size(*addr);
}

inline sivmc_bytes32 get_code_hash(sivmc_host_context* h, const sivmc_address* addr) noexcept
{
    return Host::from_context(h)->get_code_hash(*addr);
}

inline size_t copy_code(sivmc_host_context* h,
                        const sivmc_address* addr,
                        size_t code_offset,
                        uint8_t* buffer_data,
                        size_t buffer_size) noexcept
{
    return Host::from_context(h)->copy_code(*addr, code_offset, buffer_data, buffer_size);
}

inline bool selfdestruct(sivmc_host_context* h,
                         const sivmc_address* addr,
                         const sivmc_address* beneficiary) noexcept
{
    return Host::from_context(h)->selfdestruct(*addr, *beneficiary);
}

inline sivmc_result call(sivmc_host_context* h, const sivmc_message* msg) noexcept
{
    return Host::from_context(h)->call(*msg).release_raw();
}

inline sivmc_tx_context get_tx_context(sivmc_host_context* h) noexcept
{
    return Host::from_context(h)->get_tx_context();
}

inline sivmc_bytes32 get_block_hash(sivmc_host_context* h, int64_t block_number) noexcept
{
    return Host::from_context(h)->get_block_hash(block_number);
}

inline void emit_log(sivmc_host_context* h,
                     const sivmc_address* addr,
                     const uint8_t* data,
                     size_t data_size,
                     const sivmc_bytes32 topics[],
                     size_t num_topics) noexcept
{
    Host::from_context(h)->emit_log(*addr, data, data_size, static_cast<const bytes32*>(topics),
                                    num_topics);
}

inline sivmc_access_status access_account(sivmc_host_context* h, const sivmc_address* addr) noexcept
{
    return Host::from_context(h)->access_account(*addr);
}

inline sivmc_access_status access_storage(sivmc_host_context* h,
                                          const sivmc_address* addr,
                                          const sivmc_bytes32* key) noexcept
{
    return Host::from_context(h)->access_storage(*addr, *key);
}

inline sivmc_bytes32 get_transient_storage(sivmc_host_context* h,
                                           const sivmc_address* addr,
                                           const sivmc_bytes32* key) noexcept
{
    return Host::from_context(h)->get_transient_storage(*addr, *key);
}

inline void set_transient_storage(sivmc_host_context* h,
                                  const sivmc_address* addr,
                                  const sivmc_bytes32* key,
                                  const sivmc_bytes32* value) noexcept
{
    Host::from_context(h)->set_transient_storage(*addr, *key, *value);
}
}  // namespace internal

inline const sivmc_host_interface& Host::get_interface() noexcept
{
    static constexpr sivmc_host_interface interface = {
        ::sivmc::internal::account_exists,
        ::sivmc::internal::get_storage,
        ::sivmc::internal::set_storage,
        ::sivmc::internal::get_balance,
        ::sivmc::internal::get_nonce,
        ::sivmc::internal::get_code_size,
        ::sivmc::internal::get_code_hash,
        ::sivmc::internal::copy_code,
        ::sivmc::internal::selfdestruct,
        ::sivmc::internal::call,
        ::sivmc::internal::get_tx_context,
        ::sivmc::internal::get_block_hash,
        ::sivmc::internal::emit_log,
        ::sivmc::internal::access_account,
        ::sivmc::internal::access_storage,
        ::sivmc::internal::get_transient_storage,
        ::sivmc::internal::set_transient_storage,
    };
    return interface;
}
}  // namespace sivmc


/// "Stream out" operator implementation for ::sivmc_status_code.
///
/// @note This is defined in global namespace to match ::sivmc_status_code definition and allow
///       convenient operator overloading usage.
inline std::ostream& operator<<(std::ostream& os, sivmc_status_code status_code)
{
    return os << sivmc::to_string(status_code);
}

/// "Stream out" operator implementation for ::sivmc_revision.
///
/// @note This is defined in global namespace to match ::sivmc_revision definition and allow
///       convenient operator overloading usage.
inline std::ostream& operator<<(std::ostream& os, sivmc_revision rev)
{
    return os << sivmc::to_string(rev);
}

namespace std
{
/// Hash operator template specialization for sivmc::address. Needed for unordered containers.
template <>
struct hash<sivmc::address>
{
    /// Hash operator using FNV1a-based folding.
    constexpr size_t operator()(const sivmc::address& s) const noexcept
    {
        using namespace sivmc;
        using namespace fnv;
        return static_cast<size_t>(fnv1a_by64(
            fnv1a_by64(fnv1a_by64(fnv::offset_basis, load64le(&s.bytes[0])), load64le(&s.bytes[8])),
            load32le(&s.bytes[16])));
    }
};

/// Hash operator template specialization for sivmc::bytes32. Needed for unordered containers.
template <>
struct hash<sivmc::bytes32>
{
    /// Hash operator using FNV1a-based folding.
    constexpr size_t operator()(const sivmc::bytes32& s) const noexcept
    {
        using namespace sivmc;
        using namespace fnv;
        return static_cast<size_t>(
            fnv1a_by64(fnv1a_by64(fnv1a_by64(fnv1a_by64(fnv::offset_basis, load64le(&s.bytes[0])),
                                             load64le(&s.bytes[8])),
                                  load64le(&s.bytes[16])),
                       load64le(&s.bytes[24])));
    }
};
}  // namespace std
