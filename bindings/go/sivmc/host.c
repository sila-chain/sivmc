// SIVMC: Sila VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include "_cgo_export.h"

#include <stdlib.h>

/* Go does not support exporting functions with parameters with const modifiers,
 * so we have to cast function pointers to the function types defined in SIVMC.
 * This disables any type checking of exported Go functions. To mitigate this
 * problem the go_exported_functions_type_checks() function simulates usage
 * of Go exported functions with expected types to check them during compilation.
 */
const struct sivmc_host_interface sivmc_go_host = {
    (sivmc_account_exists_fn)accountExists,
    (sivmc_get_storage_fn)getStorage,
    (sivmc_set_storage_fn)setStorage,
    (sivmc_get_balance_fn)getBalance,
    (sivmc_get_nonce_fn)getNonce,
    (sivmc_get_code_size_fn)getCodeSize,
    (sivmc_get_code_hash_fn)getCodeHash,
    (sivmc_copy_code_fn)copyCode,
    (sivmc_selfdestruct_fn)selfdestruct,
    (sivmc_call_fn)call,
    (sivmc_get_tx_context_fn)getTxContext,
    (sivmc_get_block_hash_fn)getBlockHash,
    (sivmc_emit_log_fn)emitLog,
    (sivmc_access_account_fn)accessAccount,
    (sivmc_access_storage_fn)accessStorage,
    (sivmc_get_transient_storage_fn)getTransientStorage,
    (sivmc_set_transient_storage_fn)setTransientStorage,
};


#pragma GCC diagnostic error "-Wconversion"
static inline void go_exported_functions_type_checks()
{
    struct sivmc_host_context* context = NULL;
    sivmc_address* address = NULL;
    sivmc_bytes32 bytes32;
    uint8_t* data = NULL;
    size_t size = 0;
    int64_t number = 0;
    uint64_t nonce = 0;
    (void)nonce;
    struct sivmc_message* message = NULL;

    sivmc_uint256be uint256be;
    (void)uint256be;
    struct sivmc_tx_context tx_context;
    (void)tx_context;
    struct sivmc_result result;
    (void)result;
    enum sivmc_access_status access_status;
    (void)access_status;
    enum sivmc_storage_status storage_status;
    (void)storage_status;
    bool bool_flag;
    (void)bool_flag;

    sivmc_account_exists_fn account_exists_fn = NULL;
    bool_flag = account_exists_fn(context, address);
    bool_flag = accountExists(context, address);

    sivmc_get_storage_fn get_storage_fn = NULL;
    bytes32 = get_storage_fn(context, address, &bytes32);
    bytes32 = getStorage(context, address, &bytes32);

    sivmc_set_storage_fn set_storage_fn = NULL;
    storage_status = set_storage_fn(context, address, &bytes32, &bytes32);
    storage_status = setStorage(context, address, &bytes32, &bytes32);

    sivmc_get_balance_fn get_balance_fn = NULL;
    uint256be = get_balance_fn(context, address);
    uint256be = getBalance(context, address);

    sivmc_get_nonce_fn get_nonce_fn = NULL;
    nonce = get_nonce_fn(context, address);
    nonce = getNonce(context, address);

    sivmc_get_code_size_fn get_code_size_fn = NULL;
    size = get_code_size_fn(context, address);
    size = getCodeSize(context, address);

    sivmc_get_code_hash_fn get_code_hash_fn = NULL;
    bytes32 = get_code_hash_fn(context, address);
    bytes32 = getCodeHash(context, address);

    sivmc_copy_code_fn copy_code_fn = NULL;
    size = copy_code_fn(context, address, size, data, size);
    size = copyCode(context, address, size, data, size);

    sivmc_selfdestruct_fn selfdestruct_fn = NULL;
    bool_flag = selfdestruct_fn(context, address, address);
    bool_flag = selfdestruct(context, address, address);

    sivmc_call_fn call_fn = NULL;
    result = call_fn(context, message);
    result = call(context, message);

    sivmc_get_tx_context_fn get_tx_context_fn = NULL;
    tx_context = get_tx_context_fn(context);
    tx_context = getTxContext(context);

    sivmc_get_block_hash_fn get_block_hash_fn = NULL;
    bytes32 = get_block_hash_fn(context, number);
    bytes32 = getBlockHash(context, number);

    sivmc_emit_log_fn emit_log_fn = NULL;
    emit_log_fn(context, address, data, size, &bytes32, size);
    emitLog(context, address, data, size, &bytes32, size);

    sivmc_access_account_fn access_account_fn = NULL;
    access_status = access_account_fn(context, address);
    access_status = accessAccount(context, address);

    sivmc_access_storage_fn access_storage_fn = NULL;
    access_status = access_storage_fn(context, address, &bytes32);
    access_status = accessStorage(context, address, &bytes32);
}
