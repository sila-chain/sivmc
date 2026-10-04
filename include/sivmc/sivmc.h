/**
 * SIVMC: Sila VM Connector API
 *
 * @copyright
 * Copyright 2016 The EVMC Authors.
 * Licensed under the Apache License, Version 2.0.
 *
 * @defgroup SIVMC SIVMC
 * @{
 */
#ifndef SIVMC_H
#define SIVMC_H

#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 6)
/**
 * Portable declaration of "deprecated" attribute.
 *
 * Available for clang and GCC 6+ compilers. The older GCC compilers know
 * this attribute, but it cannot be applied to enum elements.
 */
#define SIVMC_DEPRECATED __attribute__((deprecated))
#else
#define SIVMC_DEPRECATED
#endif


#include <stdbool.h> /* Definition of bool, true and false. */
#include <stddef.h>  /* Definition of size_t. */
#include <stdint.h>  /* Definition of int64_t, uint64_t. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Zero default member initializer, applied in C++ only (C has no default member initializers).
 */
#ifdef __cplusplus
#define SIVMC_ZERO_INIT = 0
#else
#define SIVMC_ZERO_INIT
#endif

/* BEGIN Python CFFI declarations */

enum
{
    /**
     * The SIVMC ABI version number of the interface declared in this file.
     *
     * The ABI version is incremented on every incompatible change of the SIVMC API or ABI
     * (up to ABI version 12 it equaled the major version number of the project).
     * The Host SHOULD check if the ABI versions match when dynamically loading VMs.
     *
     * @see @ref versioning
     */
    SIVMC_ABI_VERSION = 19
};


/**
 * The fixed size array of 32 bytes.
 *
 * 32 bytes of data capable of storing e.g. 256-bit hashes.
 */
typedef struct sivmc_bytes32
{
    /** The 32 bytes. */
    uint8_t bytes[32];
} sivmc_bytes32;

/**
 * The alias for sivmc_bytes32 to represent a big-endian 256-bit integer.
 */
typedef struct sivmc_bytes32 sivmc_uint256be;

/** Big-endian 160-bit hash suitable for keeping a Sila address. */
typedef struct sivmc_address
{
    /** The 20 bytes of the hash. */
    uint8_t bytes[20];
} sivmc_address;

/** The kind of call-like instruction. */
enum sivmc_call_kind
{
    SIVMC_CALL = 0,         /**< Request CALL. */
    SIVMC_DELEGATECALL = 1, /**< Request DELEGATECALL. Valid since SilaHomestead.
                                The value param ignored. */
    SIVMC_CALLCODE = 2,     /**< Request CALLCODE. */
    SIVMC_CREATE = 3,       /**< Request CREATE. */
    SIVMC_CREATE2 = 4,      /**< Request CREATE2. Valid since SilaConstantinopleFix. */
};

/** The flags for ::sivmc_message. */
enum sivmc_flags
{
    SIVMC_STATIC = 1,   /**< Static call mode. */
    SIVMC_DELEGATED = 2 /**< Delegated call mode (SIP-7702). Valid since SilaPrague. */
};

/**
 * The message describing a Sivm call, including a zero-depth calls from a transaction origin.
 *
 * Most of the fields are modelled by the section 8. Message Call of the Sila Yellow Paper.
 */
struct sivmc_message
{
    /** The kind of the call. For zero-depth calls ::SIVMC_CALL SHOULD be used. */
    enum sivmc_call_kind kind;

    /**
     * Additional flags modifying the call execution behavior.
     *
     */
    uint32_t flags;

    /**
     * The present depth of the message call stack.
     *
     * Defined as `e` in the Yellow Paper.
     */
    int32_t depth;

    /**
     * The amount of gas available to the message execution.
     *
     * Defined as `g` in the Yellow Paper.
     */
    int64_t gas;

    /**
     * The amount of state gas available (SIP-8037).
     */
    int64_t state_gas;

    /**
     * The recipient of the message.
     *
     * This is the address of the account which storage/balance/nonce is going to be modified
     * by the message execution. In case of ::SIVMC_CALL, this is also the account where the
     * message value sivmc_message::value is going to be transferred.
     * For ::SIVMC_CALLCODE or ::SIVMC_DELEGATECALL, this may be different from
     * the sivmc_message::code_address.
     * For ::SIVMC_CREATE and ::SIVMC_CREATE2 this is the address of the account to be created.
     *
     * Defined as `r` in the Yellow Paper.
     */
    sivmc_address recipient;

    /**
     * The sender of the message.
     *
     * The address of the sender of a message call defined as `s` in the Yellow Paper.
     * This must be the message recipient of the message at the previous (lower) depth,
     * except for the ::SIVMC_DELEGATECALL where recipient is the 2 levels above the present depth.
     * At the depth 0 this must be the transaction origin.
     */
    sivmc_address sender;

    /**
     * The message input data.
     *
     * The arbitrary length byte array of the input data of the call,
     * defined as `d` in the Yellow Paper.
     * This MAY be NULL.
     */
    const uint8_t* input_data;

    /**
     * The size of the message input data.
     *
     * If input_data is NULL this MUST be 0.
     */
    size_t input_size;

    /**
     * The amount of SIL transferred with the message.
     *
     * This is transferred value for ::SIVMC_CALL or apparent value for ::SIVMC_DELEGATECALL.
     * Defined as `v` or `v~` in the Yellow Paper.
     */
    sivmc_uint256be value;

    /**
     * The address of the code to be executed.
     *
     * For ::SIVMC_CALLCODE or ::SIVMC_DELEGATECALL this may be different from
     * the sivmc_message::recipient.
     * Not required when invoking sivmc_execute_fn(), only when invoking sivmc_call_fn().
     * Ignored if kind is ::SIVMC_CREATE or ::SIVMC_CREATE2.
     *
     * Defined as `c` in the Yellow Paper.
     */
    sivmc_address code_address;

    /**
     * The code to be executed.
     */
    const uint8_t* code;

    /**
     * The length of the code to be executed.
     */
    size_t code_size;
};

/** The transaction and block data for execution. */
struct sivmc_tx_context
{
    sivmc_uint256be tx_gas_price;      /**< The transaction gas price. */
    sivmc_address tx_origin;           /**< The transaction origin account. */
    sivmc_address block_coinbase;      /**< The miner of the block. */
    int64_t block_number;              /**< The block number. */
    int64_t block_timestamp;           /**< The block timestamp. */
    int64_t block_gas_limit;           /**< The block gas limit. */
    sivmc_uint256be block_prev_randao; /**< The block previous RANDAO (SIP-4399). */
    sivmc_uint256be chain_id;          /**< The blockchain's ChainID. */
    sivmc_uint256be block_base_fee;    /**< The block base fee per gas (SIP-1559, SIP-3198). */
    sivmc_uint256be blob_base_fee;     /**< The blob base fee (SIP-7516). */
    const sivmc_bytes32* blob_hashes;  /**< The array of blob hashes (SIP-4844). */
    size_t blob_hashes_count;          /**< The number of blob hashes (SIP-4844). */
    uint64_t block_slot_number;        /**< The beacon chain slot number (SIP-7843). */
};

/**
 * @struct sivmc_host_context
 * The opaque data type representing the Host execution context.
 * @see sivmc_execute_fn().
 */
struct sivmc_host_context;

/**
 * Get transaction context callback function.
 *
 *  This callback function is used by a Sivm to retrieve the transaction and
 *  block context.
 *
 *  @param      context  The pointer to the Host execution context.
 *  @return              The transaction context.
 */
typedef struct sivmc_tx_context (*sivmc_get_tx_context_fn)(struct sivmc_host_context* context);

/**
 * Get block hash callback function.
 *
 * This callback function is used by a VM to query the hash of the header of the given block.
 * If the information about the requested block is not available, then this is signalled by
 * returning null bytes.
 *
 * @param context  The pointer to the Host execution context.
 * @param number   The block number.
 * @return         The block hash or null bytes
 *                 if the information about the block is not available.
 */
typedef sivmc_bytes32 (*sivmc_get_block_hash_fn)(struct sivmc_host_context* context,
                                                 int64_t number);

/**
 * The execution status code.
 *
 * Successful execution is represented by ::SIVMC_SUCCESS having value 0.
 *
 * Positive values represent failures defined by VM specifications with generic
 * ::SIVMC_FAILURE code of value 1.
 *
 * Status codes with negative values represent VM internal errors
 * not provided by Sivm specifications. These errors MUST not be passed back
 * to the caller. They MAY be handled by the Client in predefined manner
 * (see e.g. ::SIVMC_REJECTED), otherwise internal errors are not recoverable.
 * The generic representant of errors is ::SIVMC_INTERNAL_ERROR but
 * a Sivm implementation MAY return negative status codes that are not defined
 * in the SIVMC documentation.
 *
 * @note
 * In case new status codes are needed, please create an issue or pull request
 * in the SIVMC repository (https://github.com/sila-chain/sivmc).
 */
enum sivmc_status_code
{
    /** Execution finished with success. */
    SIVMC_SUCCESS = 0,

    /** Generic execution failure. */
    SIVMC_FAILURE = 1,

    /**
     * Execution terminated with REVERT opcode.
     *
     * In this case the amount of gas left MAY be non-zero and additional output
     * data MAY be provided in ::sivmc_result.
     */
    SIVMC_REVERT = 2,

    /** The execution has run out of gas. */
    SIVMC_OUT_OF_GAS = 3,

    /**
     * The designated INVALID instruction has been hit during execution.
     *
     * The SIP-141 (https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-141.md)
     * defines the instruction 0xfe as INVALID instruction to indicate execution
     * abortion coming from high-level languages. This status code is reported
     * in case this INVALID instruction has been encountered.
     */
    SIVMC_INVALID_INSTRUCTION = 4,

    /** An undefined instruction has been encountered. */
    SIVMC_UNDEFINED_INSTRUCTION = 5,

    /**
     * The execution has attempted to put more items on the Sivm stack
     * than the specified limit.
     */
    SIVMC_STACK_OVERFLOW = 6,

    /** Execution of an opcode has required more items on the Sivm stack. */
    SIVMC_STACK_UNDERFLOW = 7,

    /** Execution has violated the jump destination restrictions. */
    SIVMC_BAD_JUMP_DESTINATION = 8,

    /**
     * Tried to read outside memory bounds.
     *
     * An example is RETURNDATACOPY reading past the available buffer.
     */
    SIVMC_INVALID_MEMORY_ACCESS = 9,

    /** Call depth has exceeded the limit (if any) */
    SIVMC_CALL_DEPTH_EXCEEDED = 10,

    /** Tried to execute an operation which is restricted in static mode. */
    SIVMC_STATIC_MODE_VIOLATION = 11,

    /**
     * A call to a precompiled or system contract has ended with a failure.
     *
     * An example: elliptic curve functions handed invalid EC points.
     */
    SIVMC_PRECOMPILE_FAILURE = 12,

    /**
     * Contract validation has failed (e.g. due to code validity rules).
     */
    SIVMC_CONTRACT_VALIDATION_FAILURE = 13,

    /**
     * An argument to a state accessing method has a value outside of the
     * accepted range of values.
     */
    SIVMC_ARGUMENT_OUT_OF_RANGE = 14,

    /**
     * A WebAssembly `unreachable` instruction has been hit during execution.
     */
    SIVMC_WASM_UNREACHABLE_INSTRUCTION = 15,

    /**
     * A WebAssembly trap has been hit during execution. This can be for many
     * reasons, including division by zero, validation errors, etc.
     */
    SIVMC_WASM_TRAP = 16,

    /** The caller does not have enough funds for value transfer. */
    SIVMC_INSUFFICIENT_BALANCE = 17,

    /** Sivm implementation generic internal error. */
    SIVMC_INTERNAL_ERROR = -1,

    /**
     * The execution of the given code and/or message has been rejected
     * by the Sivm implementation.
     *
     * This error SHOULD be used to signal that the Sivm is not able to or
     * willing to execute the given code type or message.
     * If a Sivm returns the ::SIVMC_REJECTED status code,
     * the Client MAY try to execute it in other Sivm implementation.
     * For example, the Client tries running a code in the Sivm 1.5. If the
     * code is not supported there, the execution falls back to the Sivm 1.0.
     */
    SIVMC_REJECTED = -2,

    /** The VM failed to allocate the amount of memory needed for execution. */
    SIVMC_OUT_OF_MEMORY = -3
};

/* Forward declaration. */
struct sivmc_result;

/**
 * Releases resources assigned to an execution result.
 *
 * This function releases memory (and other resources, if any) assigned to the
 * specified execution result making the result object invalid.
 *
 * @param result  The execution result which resources are to be released. The
 *                result itself it not modified by this function, but becomes
 *                invalid and user MUST discard it as well.
 *                This MUST NOT be NULL.
 *
 * @note
 * The result is passed by pointer to avoid (shallow) copy of the ::sivmc_result
 * struct. Think of this as the best possible C language approximation to
 * passing objects by reference.
 */
typedef void (*sivmc_release_result_fn)(const struct sivmc_result* result);

/** The state-gas counters of an execution (SIP-8037). */
struct sivmc_state_gas
{
    /** The amount of state gas left. */
    int64_t left SIVMC_ZERO_INIT;

    /** The portion of the consumed state gas taken from sivmc_result::gas_left. */
    int64_t spilled SIVMC_ZERO_INIT;
};

/** The Sivm code execution result. */
struct sivmc_result
{
    /** The execution status code. */
    enum sivmc_status_code status_code;

    /**
     * The amount of gas left after the execution.
     *
     * If sivmc_result::status_code is neither ::SIVMC_SUCCESS nor ::SIVMC_REVERT
     * the value MUST be 0.
     */
    int64_t gas_left;

    /**
     * The refunded gas accumulated from this execution and its sub-calls.
     *
     * The transaction gas refund limit is not applied.
     * If sivmc_result::status_code is other than ::SIVMC_SUCCESS the value MUST be 0.
     */
    int64_t gas_refund;

    /**
     * The state-gas counters after execution (SIP-8037).
     *
     * If sivmc_result::status_code is other than ::SIVMC_SUCCESS, sivmc_state_gas::left MUST equal
     * ::sivmc_message::state_gas and sivmc_state_gas::spilled MUST be 0. The VM returns the spill
     * to sivmc_result::gas_left for ::SIVMC_REVERT; any other failure consumes it with gas_left.
     */
    struct sivmc_state_gas state_gas;

    /**
     * The reference to output data.
     *
     * The output contains data coming from RETURN opcode (iff sivmc_result::code
     * field is ::SIVMC_SUCCESS) or from REVERT opcode.
     *
     * The memory containing the output data is owned by Sivm and has to be
     * freed with sivmc_result::release().
     *
     * This pointer MAY be NULL.
     * If sivmc_result::output_size is 0 this pointer MUST NOT be dereferenced.
     */
    const uint8_t* output_data;

    /**
     * The size of the output data.
     *
     * If sivmc_result::output_data is NULL this MUST be 0.
     */
    size_t output_size;

    /**
     * The method releasing all resources associated with the result object.
     *
     * This method (function pointer) is optional (MAY be NULL) and MAY be set
     * by the VM implementation. If set it MUST be called by the user once to
     * release memory and other resources associated with the result object.
     * Once the resources are released the result object MUST NOT be used again.
     *
     * The suggested code pattern for releasing execution results:
     * @code
     * struct sivmc_result result = ...;
     * if (result.release)
     *     result.release(&result);
     * @endcode
     *
     * @note
     * It works similarly to C++ virtual destructor. Attaching the release
     * function to the result itself allows VM composition.
     */
    sivmc_release_result_fn release;
};


/**
 * Check account existence callback function.
 *
 * This callback function is used by the VM to check if
 * there exists an account at given address.
 * @param context  The pointer to the Host execution context.
 * @param address  The address of the account the query is about.
 * @return         true if exists, false otherwise.
 */
typedef bool (*sivmc_account_exists_fn)(struct sivmc_host_context* context,
                                        const sivmc_address* address);

/**
 * Get storage callback function.
 *
 * This callback function is used by a VM to query the given account storage entry.
 *
 * @param context  The Host execution context.
 * @param address  The address of the account.
 * @param key      The index of the account's storage entry.
 * @return         The storage value at the given storage key or null bytes
 *                 if the account does not exist.
 */
typedef sivmc_bytes32 (*sivmc_get_storage_fn)(struct sivmc_host_context* context,
                                              const sivmc_address* address,
                                              const sivmc_bytes32* key);

/**
 * Get transient storage callback function.
 *
 * This callback function is used by a VM to query
 * the given account transient storage (SIP-1153) entry.
 *
 * @param context  The Host execution context.
 * @param address  The address of the account.
 * @param key      The index of the account's transient storage entry.
 * @return         The transient storage value at the given storage key or null bytes
 *                 if the account does not exist.
 */
typedef sivmc_bytes32 (*sivmc_get_transient_storage_fn)(struct sivmc_host_context* context,
                                                        const sivmc_address* address,
                                                        const sivmc_bytes32* key);


/**
 * The effect of an attempt to modify a contract storage item.
 *
 * See @ref storagestatus for additional information about design of this enum
 * and analysis of the specification.
 *
 * For the purpose of explaining the meaning of each element, the following
 * notation is used:
 * - 0 is zero value,
 * - X != 0 (X is any value other than 0),
 * - Y != 0, Y != X,  (Y is any value other than X and 0),
 * - Z != 0, Z != X, Z != X (Z is any value other than Y and X and 0),
 * - the "o -> c -> v" triple describes the change status in the context of:
 *   - o: original value (cold value before a transaction started),
 *   - c: current storage value,
 *   - v: new storage value to be set.
 *
 * The order of elements follows SIPs introducing net storage gas costs:
 * - SIP-2200: https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-2200.md,
 * - SIP-1283: https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-1283.md.
 */
enum sivmc_storage_status
{
    /**
     * The new/same value is assigned to the storage item without affecting the cost structure.
     *
     * The storage value item is either:
     * - left unchanged (c == v) or
     * - the dirty value (o != c) is modified again (c != v).
     * This is the group of cases related to minimal gas cost of only accessing warm storage.
     * 0|X   -> 0 -> 0 (current value unchanged)
     * 0|X|Y -> Y -> Y (current value unchanged)
     * 0|X   -> Y -> Z (modified previously added/modified value)
     *
     * This is "catch all remaining" status. I.e. if all other statuses are correctly matched
     * this status should be assigned to all remaining cases.
     */
    SIVMC_STORAGE_ASSIGNED = 0,

    /**
     * A new storage item is added by changing
     * the current clean zero to a nonzero value.
     * 0 -> 0 -> Z
     */
    SIVMC_STORAGE_ADDED = 1,

    /**
     * A storage item is deleted by changing
     * the current clean nonzero to the zero value.
     * X -> X -> 0
     */
    SIVMC_STORAGE_DELETED = 2,

    /**
     * A storage item is modified by changing
     * the current clean nonzero to other nonzero value.
     * X -> X -> Z
     */
    SIVMC_STORAGE_MODIFIED = 3,

    /**
     * A storage item is added by changing
     * the current dirty zero to a nonzero value other than the original value.
     * X -> 0 -> Z
     */
    SIVMC_STORAGE_DELETED_ADDED = 4,

    /**
     * A storage item is deleted by changing
     * the current dirty nonzero to the zero value and the original value is not zero.
     * X -> Y -> 0
     */
    SIVMC_STORAGE_MODIFIED_DELETED = 5,

    /**
     * A storage item is added by changing
     * the current dirty zero to the original value.
     * X -> 0 -> X
     */
    SIVMC_STORAGE_DELETED_RESTORED = 6,

    /**
     * A storage item is deleted by changing
     * the current dirty nonzero to the original zero value.
     * 0 -> Y -> 0
     */
    SIVMC_STORAGE_ADDED_DELETED = 7,

    /**
     * A storage item is modified by changing
     * the current dirty nonzero to the original nonzero value other than the current value.
     * X -> Y -> X
     */
    SIVMC_STORAGE_MODIFIED_RESTORED = 8
};


/**
 * Set storage callback function.
 *
 * This callback function is used by a VM to update the given account storage entry.
 * The VM MUST make sure that the account exists. This requirement is only a formality because
 * VM implementations only modify storage of the account of the current execution context
 * (i.e. referenced by sivmc_message::recipient).
 *
 * @param context  The pointer to the Host execution context.
 * @param address  The address of the account.
 * @param key      The index of the storage entry.
 * @param value    The value to be stored.
 * @return         The effect on the storage item.
 */
typedef enum sivmc_storage_status (*sivmc_set_storage_fn)(struct sivmc_host_context* context,
                                                          const sivmc_address* address,
                                                          const sivmc_bytes32* key,
                                                          const sivmc_bytes32* value);

/**
 * Set transient storage callback function.
 *
 * This callback function is used by a VM to update
 * the given account's transient storage (SIP-1153) entry.
 * The VM MUST make sure that the account exists. This requirement is only a formality because
 * VM implementations only modify storage of the account of the current execution context
 * (i.e. referenced by sivmc_message::recipient).
 *
 * @param context  The pointer to the Host execution context.
 * @param address  The address of the account.
 * @param key      The index of the transient storage entry.
 * @param value    The value to be stored.
 */
typedef void (*sivmc_set_transient_storage_fn)(struct sivmc_host_context* context,
                                               const sivmc_address* address,
                                               const sivmc_bytes32* key,
                                               const sivmc_bytes32* value);

/**
 * Get balance callback function.
 *
 * This callback function is used by a VM to query the balance of the given account.
 *
 * @param context  The pointer to the Host execution context.
 * @param address  The address of the account.
 * @return         The balance of the given account or 0 if the account does not exist.
 */
typedef sivmc_uint256be (*sivmc_get_balance_fn)(struct sivmc_host_context* context,
                                                const sivmc_address* address);

/**
 * Get nonce callback function.
 *
 * This callback function is used by a VM to query the nonce of the given account in the state.
 *
 * @param context  The pointer to the Host execution context.
 * @param address  The address of the account.
 * @return         The nonce of the given account or 0 if the account does not exist.
 */
typedef uint64_t (*sivmc_get_nonce_fn)(struct sivmc_host_context* context,
                                       const sivmc_address* address);

/**
 * Get code size callback function.
 *
 * This callback function is used by a VM to get the size of the code stored
 * in the account at the given address.
 *
 * @param context  The pointer to the Host execution context.
 * @param address  The address of the account.
 * @return         The size of the code in the account or 0 if the account does not exist.
 */
typedef size_t (*sivmc_get_code_size_fn)(struct sivmc_host_context* context,
                                         const sivmc_address* address);

/**
 * Get code hash callback function.
 *
 * This callback function is used by a VM to get the keccak256 hash of the code stored
 * in the account at the given address. For existing accounts not having a code, this
 * function returns keccak256 hash of empty data.
 *
 * @param context  The pointer to the Host execution context.
 * @param address  The address of the account.
 * @return         The hash of the code in the account or null bytes if the account does not exist.
 */
typedef sivmc_bytes32 (*sivmc_get_code_hash_fn)(struct sivmc_host_context* context,
                                                const sivmc_address* address);

/**
 * Copy code callback function.
 *
 * This callback function is used by a Sivm to request a copy of the code
 * of the given account to the memory buffer provided by the Sivm.
 * The Client MUST copy the requested code, starting with the given offset,
 * to the provided memory buffer up to the size of the buffer or the size of
 * the code, whichever is smaller.
 *
 * @param context      The pointer to the Host execution context. See ::sivmc_host_context.
 * @param address      The address of the account.
 * @param code_offset  The offset of the code to copy.
 * @param buffer_data  The pointer to the memory buffer allocated by the Sivm
 *                     to store a copy of the requested code.
 * @param buffer_size  The size of the memory buffer.
 * @return             The number of bytes copied to the buffer by the Client.
 */
typedef size_t (*sivmc_copy_code_fn)(struct sivmc_host_context* context,
                                     const sivmc_address* address,
                                     size_t code_offset,
                                     uint8_t* buffer_data,
                                     size_t buffer_size);

/**
 * Selfdestruct callback function.
 *
 * This callback function is used by a Sivm to SELFDESTRUCT given contract.
 * The execution of the contract will not be stopped, that is up to the Sivm.
 *
 * @param context      The pointer to the Host execution context. See ::sivmc_host_context.
 * @param address      The address of the contract to be selfdestructed.
 * @param beneficiary  The address where the remaining ETH is going to be transferred.
 * @return             The information if the given address has not been registered as
 *                     selfdestructed yet. True if registered for the first time, false otherwise.
 */
typedef bool (*sivmc_selfdestruct_fn)(struct sivmc_host_context* context,
                                      const sivmc_address* address,
                                      const sivmc_address* beneficiary);

/**
 * Log callback function.
 *
 * This callback function is used by a Sivm to inform about a LOG that happened
 * during a Sivm bytecode execution.
 *
 * @param context       The pointer to the Host execution context. See ::sivmc_host_context.
 * @param address       The address of the contract that generated the log.
 * @param data          The pointer to unindexed data attached to the log.
 * @param data_size     The length of the data.
 * @param topics        The pointer to the array of topics attached to the log.
 * @param topics_count  The number of the topics. Valid values are between 0 and 4 inclusively.
 */
typedef void (*sivmc_emit_log_fn)(struct sivmc_host_context* context,
                                  const sivmc_address* address,
                                  const uint8_t* data,
                                  size_t data_size,
                                  const sivmc_bytes32 topics[],
                                  size_t topics_count);

/**
 * Access status per SIP-2929: Gas cost increases for state access opcodes.
 */
enum sivmc_access_status
#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
    : bool
#endif
{
    /**
     * The entry hasn't been accessed before – it's the first access.
     */
    SIVMC_ACCESS_COLD,

    /**
     * The entry is already in accessed_addresses or accessed_storage_keys.
     */
    SIVMC_ACCESS_WARM
};

/**
 * Access account callback function.
 *
 * This callback function is used by a VM to add the given address
 * to accessed_addresses substate (SIP-2929).
 *
 * @param context  The Host execution context.
 * @param address  The address of the account.
 * @return         SIVMC_ACCESS_WARM if accessed_addresses already contained the address
 *                 or SIVMC_ACCESS_COLD otherwise.
 */
typedef enum sivmc_access_status (*sivmc_access_account_fn)(struct sivmc_host_context* context,
                                                            const sivmc_address* address);

/**
 * Access storage callback function.
 *
 * This callback function is used by a VM to add the given account storage entry
 * to accessed_storage_keys substate (SIP-2929).
 *
 * @param context  The Host execution context.
 * @param address  The address of the account.
 * @param key      The index of the account's storage entry.
 * @return         SIVMC_ACCESS_WARM if accessed_storage_keys already contained the key
 *                 or SIVMC_ACCESS_COLD otherwise.
 */
typedef enum sivmc_access_status (*sivmc_access_storage_fn)(struct sivmc_host_context* context,
                                                            const sivmc_address* address,
                                                            const sivmc_bytes32* key);

/**
 * Pointer to the callback function supporting Sivm calls.
 *
 * @param context  The pointer to the Host execution context.
 * @param msg      The call parameters.
 * @return         The result of the call.
 */
typedef struct sivmc_result (*sivmc_call_fn)(struct sivmc_host_context* context,
                                             const struct sivmc_message* msg);

/**
 * The Host interface.
 *
 * The set of all callback functions expected by VM instances. This is C
 * realisation of vtable for OOP interface (only virtual methods, no data).
 * Host implementations SHOULD create constant singletons of this (similarly
 * to vtables) to lower the maintenance and memory management cost.
 */
struct sivmc_host_interface
{
    /** Check account existence callback function. */
    sivmc_account_exists_fn account_exists;

    /** Get storage callback function. */
    sivmc_get_storage_fn get_storage;

    /** Set storage callback function. */
    sivmc_set_storage_fn set_storage;

    /** Get balance callback function. */
    sivmc_get_balance_fn get_balance;

    /** Get nonce callback function. */
    sivmc_get_nonce_fn get_nonce;

    /** Get code size callback function. */
    sivmc_get_code_size_fn get_code_size;

    /** Get code hash callback function. */
    sivmc_get_code_hash_fn get_code_hash;

    /** Copy code callback function. */
    sivmc_copy_code_fn copy_code;

    /** Selfdestruct callback function. */
    sivmc_selfdestruct_fn selfdestruct;

    /** Call callback function. */
    sivmc_call_fn call;

    /** Get transaction context callback function. */
    sivmc_get_tx_context_fn get_tx_context;

    /** Get block hash callback function. */
    sivmc_get_block_hash_fn get_block_hash;

    /** Emit log callback function. */
    sivmc_emit_log_fn emit_log;

    /** Access account callback function. */
    sivmc_access_account_fn access_account;

    /** Access storage callback function. */
    sivmc_access_storage_fn access_storage;

    /** Get transient storage callback function. */
    sivmc_get_transient_storage_fn get_transient_storage;

    /** Set transient storage callback function. */
    sivmc_set_transient_storage_fn set_transient_storage;
};


/* Forward declaration. */
struct sivmc_vm;

/**
 * Destroys the VM instance.
 *
 * @param vm  The VM instance to be destroyed.
 */
typedef void (*sivmc_destroy_fn)(struct sivmc_vm* vm);

/**
 * Possible outcomes of sivmc_set_option.
 */
enum sivmc_set_option_result
{
    SIVMC_SET_OPTION_SUCCESS = 0,
    SIVMC_SET_OPTION_INVALID_NAME = 1,
    SIVMC_SET_OPTION_INVALID_VALUE = 2
};

/**
 * Configures the VM instance.
 *
 * Allows modifying options of the VM instance.
 * Options:
 * - code cache behavior: on, off, read-only, ...
 * - optimizations,
 *
 * @param vm     The VM instance to be configured.
 * @param name   The option name. NULL-terminated string. Cannot be NULL.
 * @param value  The new option value. NULL-terminated string. Cannot be NULL.
 * @return       The outcome of the operation.
 */
typedef enum sivmc_set_option_result (*sivmc_set_option_fn)(struct sivmc_vm* vm,
                                                            char const* name,
                                                            char const* value);


/**
 * Sivm revision.
 *
 * The revision of the Sivm specification based on the Sila
 * upgrade / hard fork codenames.
 */
enum sivmc_revision
{
    /**
     * The Frontier revision.
     *
     * The one Sila launched with.
     */
    SIVMC_FRONTIER,

    /**
     * The SilaHomestead revision.
     *
     * https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-606.md
     */
    SIVMC_SILA_HOMESTEAD,

    /**
     * The SIP150 revision.
     *
     * https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-608.md
     */
    SIVMC_SIP150,

    /**
     * The SIP158 revision.
     *
     * https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-607.md
     */
    SIVMC_SIP158,

    /**
     * The SilaByzantium revision.
     *
     * https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-609.md
     */
    SIVMC_SILA_BYZANTIUM,

    /**
     * The SilaConstantinopleFix revision.
     *
     * https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-1716.md
     */
    SIVMC_SILA_CONSTANTINOPLE_FIX,

    /**
     * The SilaIstanbul revision.
     *
     * https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-1679.md
     */
    SIVMC_SILA_ISTANBUL,

    /**
     * The SilaBerlin revision.
     *
     * https://github.com/sila-chain/execution-specs/blob/313dbf0f689a695a149ff981f4fcad73ba46173c/src/sila/forks/berlin/__init__.py
     */
    SIVMC_SILA_BERLIN,

    /**
     * The SilaLondon revision.
     *
     * https://github.com/sila-chain/execution-specs/blob/313dbf0f689a695a149ff981f4fcad73ba46173c/src/sila/forks/london/__init__.py
     */
    SIVMC_SILA_LONDON,

    /**
     * The SilaParis revision.
     *
     * https://github.com/sila-chain/execution-specs/blob/313dbf0f689a695a149ff981f4fcad73ba46173c/src/sila/forks/paris/__init__.py
     */
    SIVMC_SILA_PARIS,

    /**
     * The SilaShanghai revision.
     *
     * https://github.com/sila-chain/execution-specs/blob/313dbf0f689a695a149ff981f4fcad73ba46173c/src/sila/forks/shanghai/__init__.py
     */
    SIVMC_SILA_SHANGHAI,

    /**
     * The SilaCancun revision.
     *
     * https://github.com/sila-chain/execution-specs/blob/313dbf0f689a695a149ff981f4fcad73ba46173c/src/sila/forks/cancun/__init__.py
     */
    SIVMC_SILA_CANCUN,

    /**
     * The SilaPrague revision.
     *
     * https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-7600.md
     */
    SIVMC_SILA_PRAGUE,

    /**
     * The SilaOsaka revision.
     *
     * https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-7607.md
     */
    SIVMC_SILA_OSAKA,

    /**
     * The SilaAmsterdam revision.
     *
     * https://github.com/sila-chain/SIPs/blob/main/SIPS/sip-7773.md
     */
    SIVMC_SILA_AMSTERDAM,

    /**
     * The unspecified Sivm revision used for Sivm implementations to expose
     * experimental features.
     */
    SIVMC_EXPERIMENTAL,

    /** The maximum Sivm revision supported. */
    SIVMC_MAX_REVISION = SIVMC_EXPERIMENTAL,

    /**
     * The latest known Sivm revision with finalized specification.
     *
     * This is handy for Sivm tools to always use the latest revision available.
     */
    SIVMC_LATEST_STABLE_REVISION = SIVMC_SILA_OSAKA
};


/**
 * Executes the given code using the input from the message.
 *
 * This function MAY be invoked multiple times for a single VM instance.
 *
 * @param vm         The VM instance. This argument MUST NOT be NULL.
 * @param host       The Host interface. This argument MUST NOT be NULL.
 * @param context    The opaque pointer to the Host execution context.
 *                   This argument MAY be NULL. The VM MUST pass the same
 *                   pointer to the methods of the @p host interface.
 *                   The VM MUST NOT dereference the pointer.
 * @param rev        The requested Sivm specification revision.
 * @param msg        The call parameters. See ::sivmc_message. This argument MUST NOT be NULL.
 * @param code       The reference to the code to be executed. This argument MAY be NULL.
 * @param code_size  The length of the code. If @p code is NULL this argument MUST be 0.
 * @return           The execution result.
 */
typedef struct sivmc_result (*sivmc_execute_fn)(struct sivmc_vm* vm,
                                                const struct sivmc_host_interface* host,
                                                struct sivmc_host_context* context,
                                                enum sivmc_revision rev,
                                                const struct sivmc_message* msg,
                                                uint8_t const* code,
                                                size_t code_size);

/**
 * The VM instance.
 *
 * Defines the base struct of the VM implementation.
 */
struct sivmc_vm
{
    /**
     * SIVMC ABI version implemented by the VM instance.
     *
     * Can be used to detect ABI incompatibilities.
     * The SIVMC ABI version represented by this file is in ::SIVMC_ABI_VERSION.
     */
    const int abi_version;

    /**
     * The name of the SIVMC VM implementation.
     *
     * It MUST be a NULL-terminated not empty string.
     * The content MUST be UTF-8 encoded (this implies ASCII encoding is also allowed).
     */
    const char* name;

    /**
     * The version of the SIVMC VM implementation, e.g. "1.2.3b4".
     *
     * It MUST be a NULL-terminated not empty string.
     * The content MUST be UTF-8 encoded (this implies ASCII encoding is also allowed).
     */
    const char* version;

    /**
     * Pointer to function destroying the VM instance.
     *
     * This is a mandatory method and MUST NOT be set to NULL.
     */
    sivmc_destroy_fn destroy;

    /**
     * Pointer to function executing a code by the VM instance.
     *
     * This is a mandatory method and MUST NOT be set to NULL.
     */
    sivmc_execute_fn execute;

    /**
     * Optional pointer to function modifying VM's options.
     *
     * If the VM does not support this feature the pointer can be NULL.
     */
    sivmc_set_option_fn set_option;
};

/* END Python CFFI declarations */

#ifdef SIVMC_DOCUMENTATION
/**
 * Example of a function creating an instance of an example Sivm implementation.
 *
 * Each Sivm implementation MUST provide a function returning a Sivm instance.
 * The function SHOULD be named `sivmc_create_<vm-name>(void)`. If the VM name contains hyphens
 * replaces them with underscores in the function names.
 *
 * @par Binaries naming convention
 * For VMs distributed as shared libraries, the name of the library SHOULD match the VM name.
 * The conventional library filename prefixes and extensions SHOULD be ignored by the Client.
 * For example, the shared library with the "beta-interpreter" implementation may be named
 * `libbeta-interpreter.so`.
 *
 * @return  The VM instance or NULL indicating instance creation failure.
 */
struct sivmc_vm* sivmc_create_example_vm(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
/** @} */
