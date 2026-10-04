// SIVMC: Sila VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

/**
 * SIVMC Helpers
 *
 * A collection of C helper functions for invoking a VM instance methods.
 * These are convenient for languages where invoking function pointers
 * is "ugly" or impossible (such as Go).
 *
 * @defgroup helpers SIVMC Helpers
 * @{
 */
#pragma once

#include <sivmc/sivmc.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#endif

/**
 * Returns true if the VM has a compatible ABI version.
 */
static inline bool sivmc_is_abi_compatible(struct sivmc_vm* vm)
{
    return vm->abi_version == SIVMC_ABI_VERSION;
}

/**
 * Returns the name of the VM.
 */
static inline const char* sivmc_vm_name(struct sivmc_vm* vm)
{
    return vm->name;
}

/**
 * Returns the version of the VM.
 */
static inline const char* sivmc_vm_version(struct sivmc_vm* vm)
{
    return vm->version;
}

/**
 * Destroys the VM instance.
 *
 * @see sivmc_destroy_fn
 */
static inline void sivmc_destroy(struct sivmc_vm* vm)
{
    vm->destroy(vm);
}

/**
 * Sets the option for the VM, if the feature is supported by the VM.
 *
 * @see sivmc_set_option_fn
 */
static inline enum sivmc_set_option_result sivmc_set_option(struct sivmc_vm* vm,
                                                            char const* name,
                                                            char const* value)
{
    if (vm->set_option)
        return vm->set_option(vm, name, value);
    return SIVMC_SET_OPTION_INVALID_NAME;
}

/**
 * Executes code in the VM instance.
 *
 * @see sivmc_execute_fn.
 */
static inline struct sivmc_result sivmc_execute(struct sivmc_vm* vm,
                                                const struct sivmc_host_interface* host,
                                                struct sivmc_host_context* context,
                                                enum sivmc_revision rev,
                                                const struct sivmc_message* msg,
                                                uint8_t const* code,
                                                size_t code_size)
{
    return vm->execute(vm, host, context, rev, msg, code, code_size);
}

/// The sivmc_result release function using free() for releasing the memory.
///
/// This function is used in the sivmc_make_result(),
/// but may be also used in other case if convenient.
///
/// @param result The result object.
static void sivmc_free_result_memory(const struct sivmc_result* result)
{
    free((uint8_t*)result->output_data);
}

/// Creates the result from the provided arguments.
///
/// The provided output is copied to memory allocated with malloc()
/// and the sivmc_result::release function is set to one invoking free().
///
/// In case of memory allocation failure, the result has all fields zeroed
/// and only sivmc_result::status_code is set to ::SIVMC_OUT_OF_MEMORY internal error.
///
/// @param status_code  The status code.
/// @param gas_left     The amount of gas left.
/// @param gas_refund   The amount of refunded gas.
/// @param output_data  The pointer to the output.
/// @param output_size  The output size.
static inline struct sivmc_result sivmc_make_result(enum sivmc_status_code status_code,
                                                    int64_t gas_left,
                                                    int64_t gas_refund,
                                                    const uint8_t* output_data,
                                                    size_t output_size)
{
#ifdef __cplusplus
    struct sivmc_result result = {};
#else
    struct sivmc_result result;
    memset(&result, 0, sizeof(result));
#endif

    if (output_size != 0)
    {
        uint8_t* buffer = (uint8_t*)malloc(output_size);

        if (!buffer)
        {
            result.status_code = SIVMC_OUT_OF_MEMORY;
            return result;
        }

        memcpy(buffer, output_data, output_size);
        result.output_data = buffer;
        result.output_size = output_size;
        result.release = sivmc_free_result_memory;
    }

    result.status_code = status_code;
    result.gas_left = gas_left;
    result.gas_refund = gas_refund;
    return result;
}

/**
 * Releases the resources allocated to the execution result.
 *
 * @param result  The result object to be released. MUST NOT be NULL.
 *
 * @see sivmc_result::release() sivmc_release_result_fn
 */
static inline void sivmc_release_result(struct sivmc_result* result)
{
    if (result->release)
        result->release(result);
}

/** Returns text representation of the ::sivmc_status_code. */
static inline const char* sivmc_status_code_to_string(enum sivmc_status_code status_code)
{
    switch (status_code)
    {
    case SIVMC_SUCCESS:
        return "success";
    case SIVMC_FAILURE:
        return "failure";
    case SIVMC_REVERT:
        return "revert";
    case SIVMC_OUT_OF_GAS:
        return "out of gas";
    case SIVMC_INVALID_INSTRUCTION:
        return "invalid instruction";
    case SIVMC_UNDEFINED_INSTRUCTION:
        return "undefined instruction";
    case SIVMC_STACK_OVERFLOW:
        return "stack overflow";
    case SIVMC_STACK_UNDERFLOW:
        return "stack underflow";
    case SIVMC_BAD_JUMP_DESTINATION:
        return "bad jump destination";
    case SIVMC_INVALID_MEMORY_ACCESS:
        return "invalid memory access";
    case SIVMC_CALL_DEPTH_EXCEEDED:
        return "call depth exceeded";
    case SIVMC_STATIC_MODE_VIOLATION:
        return "static mode violation";
    case SIVMC_PRECOMPILE_FAILURE:
        return "precompile failure";
    case SIVMC_CONTRACT_VALIDATION_FAILURE:
        return "contract validation failure";
    case SIVMC_ARGUMENT_OUT_OF_RANGE:
        return "argument out of range";
    case SIVMC_WASM_UNREACHABLE_INSTRUCTION:
        return "wasm unreachable instruction";
    case SIVMC_WASM_TRAP:
        return "wasm trap";
    case SIVMC_INSUFFICIENT_BALANCE:
        return "insufficient balance";
    case SIVMC_INTERNAL_ERROR:
        return "internal error";
    case SIVMC_REJECTED:
        return "rejected";
    case SIVMC_OUT_OF_MEMORY:
        return "out of memory";
    }
    return "<unknown>";
}

/** Returns the name of the ::sivmc_revision. */
static inline const char* sivmc_revision_to_string(enum sivmc_revision rev)
{
    switch (rev)
    {
    case SIVMC_FRONTIER:
        return "Frontier";
    case SIVMC_SILA_HOMESTEAD:
        return "SilaHomestead";
    case SIVMC_SIP150:
        return "SIP150";
    case SIVMC_SIP158:
        return "SIP158";
    case SIVMC_SILA_BYZANTIUM:
        return "SilaByzantium";
    case SIVMC_SILA_CONSTANTINOPLE_FIX:
        return "SilaConstantinopleFix";
    case SIVMC_SILA_ISTANBUL:
        return "SilaIstanbul";
    case SIVMC_SILA_BERLIN:
        return "SilaBerlin";
    case SIVMC_SILA_LONDON:
        return "SilaLondon";
    case SIVMC_SILA_PARIS:
        return "SilaParis";
    case SIVMC_SILA_SHANGHAI:
        return "SilaShanghai";
    case SIVMC_SILA_CANCUN:
        return "SilaCancun";
    case SIVMC_SILA_PRAGUE:
        return "SilaPrague";
    case SIVMC_SILA_OSAKA:
        return "SilaOsaka";
    case SIVMC_SILA_AMSTERDAM:
        return "SilaAmsterdam";
    case SIVMC_EXPERIMENTAL:
        return "Experimental";
    }
    return "<unknown>";
}

/** @} */

#ifdef __cplusplus
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}  // extern "C"
#endif
