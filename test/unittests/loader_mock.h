// SIVMC: Sila VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

/**
 * @file
 * The loader OS mock for opening DLLs. To be inserted in loader.c for unit tests.
 */

static const int magic_handle = 0xE7AC;

const char* sivmc_test_library_path = NULL;
const char* sivmc_test_library_symbol = NULL;
sivmc_create_fn sivmc_test_create_fn = NULL;

static const char* sivmc_test_last_error_msg = NULL;

/* Limited variant of strcpy_s(). Exposed to unittests when building with SIVMC_LOADER_MOCK. */
int strcpy_sx(char* dest, size_t destsz, const char* src);

static int sivmc_test_load_library(const char* filename)
{
    sivmc_test_last_error_msg = NULL;
    if (filename && sivmc_test_library_path && strcmp(filename, sivmc_test_library_path) == 0)
        return magic_handle;
    sivmc_test_last_error_msg = "cannot load library";
    return 0;
}

static void sivmc_test_free_library(int handle)
{
    (void)handle;
}

static sivmc_create_fn sivmc_test_get_symbol_address(int handle, const char* symbol)
{
    if (handle != magic_handle)
        return NULL;

    if (sivmc_test_library_symbol && strcmp(symbol, sivmc_test_library_symbol) == 0)
        return sivmc_test_create_fn;
    return NULL;
}

static const char* sivmc_test_get_last_error_msg(void)
{
    // Return the last error message only once.
    const char* m = sivmc_test_last_error_msg;
    sivmc_test_last_error_msg = NULL;
    return m;
}

#define DLL_HANDLE int
#define DLL_OPEN(filename) sivmc_test_load_library(filename)
#define DLL_CLOSE(handle) sivmc_test_free_library(handle)
#define DLL_GET_CREATE_FN(handle, name) sivmc_test_get_symbol_address(handle, name)
#define DLL_GET_ERROR_MSG() sivmc_test_get_last_error_msg()
