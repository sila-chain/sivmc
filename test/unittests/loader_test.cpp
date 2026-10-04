// SIVMC: Sila VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

#include <sivmc/helpers.h>
#include <sivmc/loader.h>
#include <sivmc/sivmc.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <vector>

#if _WIN32
static constexpr bool is_windows = true;
#else
static constexpr bool is_windows = false;
#endif

extern "C" {
/// Declaration of internal function defined in loader.c.
int strcpy_sx(char* dest, size_t destsz, const char* src);

/// The library path expected by mocked sivmc_test_load_library().
extern const char* sivmc_test_library_path;

/// The symbol name expected by mocked sivmc_test_get_symbol_address().
extern const char* sivmc_test_library_symbol;

/// The pointer to function returned by sivmc_test_get_symbol_address().
extern sivmc_create_fn sivmc_test_create_fn;
}

class loader : public ::testing::Test
{
protected:
    static int create_count;
    static int destroy_count;
    static std::unordered_map<std::string, std::vector<std::string>> supported_options;
    static std::vector<std::pair<std::string, std::string>> recorded_options;
    static const std::string option_name_causing_unknown_error;

    loader() noexcept
    {
        create_count = 0;
        destroy_count = 0;
        supported_options.clear();
        recorded_options.clear();
    }

    static void setup(const char* path, const char* symbol, sivmc_create_fn fn) noexcept
    {
        sivmc_test_library_path = path;
        sivmc_test_library_symbol = symbol;
        sivmc_test_create_fn = fn;
    }

    static void destroy(sivmc_vm* /*vm*/) noexcept { ++destroy_count; }

    static sivmc_set_option_result set_option(sivmc_vm* /*vm*/,
                                              const char* name,
                                              const char* value) noexcept
    {
        recorded_options.push_back({name, value});  // NOLINT

        auto it = supported_options.find(name);
        if (it == supported_options.end())
            return SIVMC_SET_OPTION_INVALID_NAME;

        if (std::find(std::begin(it->second), std::end(it->second), value) != std::end(it->second))
            return SIVMC_SET_OPTION_SUCCESS;

        if (name == option_name_causing_unknown_error)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
            return static_cast<sivmc_set_option_result>(-42);
#pragma GCC diagnostic pop
        }

        return SIVMC_SET_OPTION_INVALID_VALUE;
    }

    /// Creates a VM mock with only destroy() method.
    static sivmc_vm* create_vm_barebone()
    {
        static auto instance =
            sivmc_vm{SIVMC_ABI_VERSION, "vm_barebone", "", destroy, nullptr, nullptr};
        ++create_count;
        return &instance;
    }

    /// Creates a VM mock with ABI version different than in this project.
    static sivmc_vm* create_vm_with_wrong_abi()
    {
        constexpr auto wrong_abi_version = 1985;
        static_assert(wrong_abi_version != SIVMC_ABI_VERSION);
        static auto instance = sivmc_vm{wrong_abi_version, "", "", destroy, nullptr, nullptr};
        ++create_count;
        return &instance;
    }

    /// Creates a VM mock with optional set_option() method.
    static sivmc_vm* create_vm_with_set_option() noexcept
    {
        static auto instance =
            sivmc_vm{SIVMC_ABI_VERSION, "vm_with_set_option", "", destroy, nullptr, set_option};
        ++create_count;
        return &instance;
    }
};

int loader::create_count = 0;
int loader::destroy_count = 0;
std::unordered_map<std::string, std::vector<std::string>> loader::supported_options;
std::vector<std::pair<std::string, std::string>> loader::recorded_options;

/// The option name that will return unexpected error code from the set_option() method.
const std::string loader::option_name_causing_unknown_error{"raise_unknown"};

namespace
{
sivmc_vm* create_aaa()
{
    return reinterpret_cast<sivmc_vm*>(0xaaa);
}

sivmc_vm* create_eee_bbb()
{
    return reinterpret_cast<sivmc_vm*>(0xeeebbb);
}

sivmc_vm* create_failure()
{
    return nullptr;
}
}  // namespace

TEST_F(loader, strcpy_sx)
{
    const char input_empty[] = "";
    const char input_that_fits[] = "x";
    const char input_too_big[] = "12";
    char buf[2] = {0x0f, 0x0f};
    static_assert(sizeof(input_empty) <= sizeof(buf));
    static_assert(sizeof(input_that_fits) <= sizeof(buf));
    static_assert(sizeof(input_too_big) > sizeof(buf));

    EXPECT_EQ(strcpy_sx(buf, sizeof(buf), input_empty), 0);
    EXPECT_EQ(buf[0], 0);
    EXPECT_EQ(buf[1], 0x0f);

    EXPECT_EQ(strcpy_sx(buf, sizeof(buf), input_that_fits), 0);
    EXPECT_EQ(buf[0], 'x');
    EXPECT_EQ(buf[1], 0);

    EXPECT_NE(strcpy_sx(buf, sizeof(buf), input_too_big), 0);
    EXPECT_EQ(buf[0], 0);
}

TEST_F(loader, load_nonexistent)
{
    constexpr auto path = "nonexistent";
    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    EXPECT_TRUE(sivmc_load(path, &ec) == nullptr);
    EXPECT_EQ(ec, SIVMC_LOADER_CANNOT_OPEN);
    EXPECT_TRUE(sivmc_load(path, nullptr) == nullptr);
}

TEST_F(loader, load_long_path)
{
    const std::string path(5000, 'a');
    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    EXPECT_TRUE(sivmc_load(path.c_str(), &ec) == nullptr);
    EXPECT_STREQ(sivmc_last_error_msg(),
                 "invalid argument: file name is too long (5000, maximum allowed length is 4096)");
    EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_ARGUMENT);
    EXPECT_TRUE(sivmc_load(path.c_str(), nullptr) == nullptr);
    EXPECT_STREQ(sivmc_last_error_msg(),
                 "invalid argument: file name is too long (5000, maximum allowed length is 4096)");
    EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
}

TEST_F(loader, load_null_path)
{
    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    EXPECT_TRUE(sivmc_load(nullptr, &ec) == nullptr);
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_ARGUMENT);
    EXPECT_STREQ(sivmc_last_error_msg(), "invalid argument: file name cannot be null");
    EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
    EXPECT_TRUE(sivmc_load(nullptr, nullptr) == nullptr);
    EXPECT_STREQ(sivmc_last_error_msg(), "invalid argument: file name cannot be null");
    EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
}

TEST_F(loader, load_empty_path)
{
    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    EXPECT_TRUE(sivmc_load("", &ec) == nullptr);
    EXPECT_STREQ(sivmc_last_error_msg(), "invalid argument: file name cannot be empty");
    EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_ARGUMENT);
    EXPECT_TRUE(sivmc_load("", nullptr) == nullptr);
    EXPECT_STREQ(sivmc_last_error_msg(), "invalid argument: file name cannot be empty");
    EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
}

TEST_F(loader, load_aaa)
{
    auto paths = {
        "./aaa.sivm",
        "aaa.sivm",
        "unittests/libaaa.so",
    };

    const auto expected_vm_ptr = reinterpret_cast<sivmc_vm*>(0xaaa);

    for (auto& path : paths)
    {
        setup(path, "sivmc_create_aaa", create_aaa);
        sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
        const auto fn = sivmc_load(path, &ec);
        EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
        ASSERT_TRUE(fn != nullptr);
        EXPECT_EQ(fn(), expected_vm_ptr);
        EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
    }
}

TEST_F(loader, load_file_with_multiple_extensions)
{
    auto paths = {
        "./aaa.sivm.0.99",
        "aaa.tar.gz.so",
        "unittests/aaa.x.y.z.so",
        "unittests/aaa.1.lib",
        "unittests/aaa.1.0",
        "unittests/aaa.extextextextextextextextextextextextextextextextext",
    };

    const auto expected_vm_ptr = reinterpret_cast<sivmc_vm*>(0xaaa);

    for (auto& path : paths)
    {
        setup(path, "sivmc_create_aaa", create_aaa);
        sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
        const auto fn = sivmc_load(path, &ec);
        EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
        ASSERT_TRUE(fn != nullptr);
        EXPECT_EQ(fn(), expected_vm_ptr);
        EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
    }
}

TEST_F(loader, load_eee_bbb)
{
    setup("unittests/eee-bbb.dll", "sivmc_create_eee_bbb", create_eee_bbb);
    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto fn = sivmc_load(sivmc_test_library_path, &ec);
    const auto expected_vm_ptr = reinterpret_cast<sivmc_vm*>(0xeeebbb);
    ASSERT_TRUE(fn != nullptr);
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
    EXPECT_EQ(fn(), expected_vm_ptr);
    EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
}


TEST_F(loader, load_windows_path)
{
    auto paths = {
        "./eee-bbb.sivm",           ".\\eee-bbb.sivm",           "./unittests/eee-bbb.dll",
        "./unittests\\eee-bbb.dll", ".\\unittests\\eee-bbb.dll", ".\\unittests/eee-bbb.dll",
        "unittests\\eee-bbb.dll",
    };

    for (auto& path : paths)
    {
        const bool should_open = is_windows || std::strchr(path, '\\') == nullptr;
        setup(should_open ? path : nullptr, "sivmc_create_eee_bbb", create_eee_bbb);

        sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
        sivmc_load(path, &ec);
        if (should_open)
        {
            EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
            EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
        }
        else
        {
            EXPECT_EQ(ec, SIVMC_LOADER_CANNOT_OPEN);
            EXPECT_STREQ(sivmc_last_error_msg(), "cannot load library");
            EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
        }
    }
}

TEST_F(loader, load_symbol_not_found)
{
    auto paths = {
        "libaaa1.so",
        "eee2.so",
        "libeee3.x",
        "eee4",
        "_",
        "lib_.so",
        "unittests/double-prefix-aaa.sivm",
        "unittests/double_prefix_aaa.sivm",
    };

    for (auto& path : paths)
    {
        setup(path, "sivmc_create_aaa", create_aaa);

        sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
        EXPECT_TRUE(sivmc_load(sivmc_test_library_path, &ec) == nullptr);
        EXPECT_EQ(ec, SIVMC_LOADER_SYMBOL_NOT_FOUND);
        EXPECT_EQ(sivmc_last_error_msg(),
                  "SIVMC create function not found in " + std::string(path));
        EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
        EXPECT_TRUE(sivmc_load(sivmc_test_library_path, nullptr) == nullptr);
    }
}

TEST_F(loader, load_default_symbol)
{
    setup("default.sivmc", "sivmc_create", create_aaa);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto fn = sivmc_load(sivmc_test_library_path, &ec);
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
    EXPECT_EQ(fn, &create_aaa);

    fn = sivmc_load(sivmc_test_library_path, nullptr);
    EXPECT_EQ(fn, &create_aaa);
}

TEST_F(loader, load_and_create_failure)
{
    setup("failure.vm", "sivmc_create", create_failure);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_create(sivmc_test_library_path, &ec);
    EXPECT_TRUE(vm == nullptr);
    EXPECT_EQ(ec, SIVMC_LOADER_VM_CREATION_FAILURE);
    EXPECT_STREQ(sivmc_last_error_msg(), "creating SIVMC VM of failure.vm has failed");
    EXPECT_TRUE(sivmc_last_error_msg() == nullptr);

    vm = sivmc_load_and_create(sivmc_test_library_path, nullptr);
    EXPECT_TRUE(vm == nullptr);
    EXPECT_STREQ(sivmc_last_error_msg(), "creating SIVMC VM of failure.vm has failed");
}

TEST_F(loader, load_and_create_abi_mismatch)
{
    setup("abi1985.vm", "sivmc_create", create_vm_with_wrong_abi);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_create(sivmc_test_library_path, &ec);
    EXPECT_TRUE(vm == nullptr);
    EXPECT_EQ(ec, SIVMC_LOADER_ABI_VERSION_MISMATCH);
    const auto expected_error_msg =
        "SIVMC ABI version 1985 of abi1985.vm mismatches the expected version " +
        std::to_string(SIVMC_ABI_VERSION);
    EXPECT_EQ(sivmc_last_error_msg(), expected_error_msg);
    EXPECT_TRUE(sivmc_last_error_msg() == nullptr);
    EXPECT_EQ(destroy_count, create_count);

    vm = sivmc_load_and_create(sivmc_test_library_path, nullptr);
    EXPECT_TRUE(vm == nullptr);
    EXPECT_EQ(sivmc_last_error_msg(), expected_error_msg);
    EXPECT_EQ(destroy_count, create_count);
}

TEST_F(loader, load_and_configure_no_options)
{
    setup("path", "sivmc_create", create_vm_with_set_option);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_configure("path", &ec);
    EXPECT_TRUE(vm);
    EXPECT_TRUE(recorded_options.empty());
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);

    setup("path", "sivmc_create", create_vm_barebone);

    vm = sivmc_load_and_configure("path,", &ec);
    EXPECT_TRUE(vm);
    EXPECT_TRUE(recorded_options.empty());
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
}

TEST_F(loader, load_and_configure_single_option)
{
    supported_options["o"] = {"1"};
    supported_options["O"] = {"2"};

    setup("path", "sivmc_create", create_vm_with_set_option);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_configure("path,o=1", &ec);
    EXPECT_TRUE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{1});
    EXPECT_EQ(recorded_options[0].first, "o");
    EXPECT_EQ(recorded_options[0].second, "1");
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);

    recorded_options.clear();
    vm = sivmc_load_and_configure("path,O=2", &ec);
    EXPECT_TRUE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{1});
    EXPECT_EQ(recorded_options[0].first, "O");
    EXPECT_EQ(recorded_options[0].second, "2");
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
}

TEST_F(loader, load_and_configure_uknown_option)
{
    supported_options["x"] = {"1"};

    setup("path", "sivmc_create", create_vm_with_set_option);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_configure("path,z=1", &ec);
    EXPECT_FALSE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{1});
    EXPECT_EQ(recorded_options[0].first, "z");
    EXPECT_EQ(recorded_options[0].second, "1");
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_OPTION_NAME);
    EXPECT_STREQ(sivmc_last_error_msg(), "vm_with_set_option (path): unknown option 'z'");
    EXPECT_EQ(destroy_count, create_count);

    recorded_options.clear();
    vm = sivmc_load_and_configure("path,x=2,", &ec);
    EXPECT_FALSE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{1});
    EXPECT_EQ(recorded_options[0].first, "x");
    EXPECT_EQ(recorded_options[0].second, "2");
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_OPTION_VALUE);
    EXPECT_STREQ(sivmc_last_error_msg(),
                 "vm_with_set_option (path): unsupported value '2' for option 'x'");
    EXPECT_EQ(destroy_count, create_count);
}

TEST_F(loader, load_and_configure_multiple_options)
{
    supported_options["a"] = {"_a", "_c"};
    supported_options["b"] = {"_b1", "_b2"};
    supported_options["c"] = {"_c"};

    setup("path", "sivmc_create", create_vm_with_set_option);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_configure("path,a=_a,b=_b1,c=_c,b=_b2", &ec);
    EXPECT_TRUE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{4});
    EXPECT_EQ(recorded_options[0].first, "a");
    EXPECT_EQ(recorded_options[0].second, "_a");
    EXPECT_EQ(recorded_options[1].first, "b");
    EXPECT_EQ(recorded_options[1].second, "_b1");
    EXPECT_EQ(recorded_options[2].first, "c");
    EXPECT_EQ(recorded_options[2].second, "_c");
    EXPECT_EQ(recorded_options[3].first, "b");
    EXPECT_EQ(recorded_options[3].second, "_b2");
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);

    recorded_options.clear();
    vm = sivmc_load_and_configure("path,a=_a,b=_b2,a=_c,", &ec);
    EXPECT_TRUE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{3});
    EXPECT_EQ(recorded_options[0].first, "a");
    EXPECT_EQ(recorded_options[0].second, "_a");
    EXPECT_EQ(recorded_options[1].first, "b");
    EXPECT_EQ(recorded_options[1].second, "_b2");
    EXPECT_EQ(recorded_options[2].first, "a");
    EXPECT_EQ(recorded_options[2].second, "_c");
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
}

TEST_F(loader, load_and_configure_uknown_option_in_sequence)
{
    supported_options["a"] = {"_a"};
    supported_options["b"] = {"_b"};
    supported_options["c"] = {"_c"};

    setup("path", "sivmc_create", create_vm_with_set_option);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_configure("path,a=_a,b=_b,c=_b,", &ec);
    EXPECT_FALSE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{3});
    EXPECT_EQ(recorded_options[0].first, "a");
    EXPECT_EQ(recorded_options[0].second, "_a");
    EXPECT_EQ(recorded_options[1].first, "b");
    EXPECT_EQ(recorded_options[1].second, "_b");
    EXPECT_EQ(recorded_options[2].first, "c");
    EXPECT_EQ(recorded_options[2].second, "_b");
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_OPTION_VALUE);
    EXPECT_STREQ(sivmc_last_error_msg(),
                 "vm_with_set_option (path): unsupported value '_b' for option 'c'");
    EXPECT_EQ(destroy_count, create_count);

    recorded_options.clear();
    vm = sivmc_load_and_configure("path,a=_a,x=_b,c=_c", &ec);
    EXPECT_FALSE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{2});
    EXPECT_EQ(recorded_options[0].first, "a");
    EXPECT_EQ(recorded_options[0].second, "_a");
    EXPECT_EQ(recorded_options[1].first, "x");
    EXPECT_EQ(recorded_options[1].second, "_b");
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_OPTION_NAME);
    EXPECT_STREQ(sivmc_last_error_msg(), "vm_with_set_option (path): unknown option 'x'");
    EXPECT_EQ(destroy_count, create_count);
}

TEST_F(loader, load_and_configure_empty_values)
{
    supported_options["flag"] = {""};  // Empty value expected.
    supported_options["e"] = {""};     // Empty value expected.

    setup("path", "sivmc_create", create_vm_with_set_option);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_configure("path,flag,e=,flag=,e", &ec);
    EXPECT_TRUE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{4});
    EXPECT_EQ(recorded_options[0].first, "flag");
    EXPECT_EQ(recorded_options[0].second, "");
    EXPECT_EQ(recorded_options[1].first, "e");
    EXPECT_EQ(recorded_options[1].second, "");
    EXPECT_EQ(recorded_options[2].first, "flag");
    EXPECT_EQ(recorded_options[2].second, "");
    EXPECT_EQ(recorded_options[3].first, "e");
    EXPECT_EQ(recorded_options[3].second, "");
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
    EXPECT_EQ(create_count, 1);
    EXPECT_EQ(destroy_count, 0);
}

TEST_F(loader, load_and_configure_degenerated_names)
{
    supported_options[""] = {"", "xxx"};  // An option with empty name.

    setup("path", "sivmc_create", create_vm_with_set_option);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_configure("path,,,=,,=xxx", &ec);
    EXPECT_TRUE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{5});
    EXPECT_EQ(recorded_options[0].first, "");
    EXPECT_EQ(recorded_options[0].second, "");
    EXPECT_EQ(recorded_options[1].first, "");
    EXPECT_EQ(recorded_options[1].second, "");
    EXPECT_EQ(recorded_options[2].first, "");
    EXPECT_EQ(recorded_options[2].second, "");
    EXPECT_EQ(recorded_options[3].first, "");
    EXPECT_EQ(recorded_options[3].second, "");
    EXPECT_EQ(recorded_options[4].first, "");
    EXPECT_EQ(recorded_options[4].second, "xxx");
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
    sivmc_destroy(vm);
    EXPECT_EQ(destroy_count, create_count);
}

TEST_F(loader, load_and_configure_comma_at_the_end)
{
    // The additional comma at the end of the configuration string is ignored.

    supported_options["x"] = {"x"};

    setup("path", "sivmc_create", create_vm_with_set_option);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_configure("path,x=x,", &ec);
    EXPECT_TRUE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{1});
    EXPECT_EQ(recorded_options[0].first, "x");
    EXPECT_EQ(recorded_options[0].second, "x");
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
    sivmc_destroy(vm);
    EXPECT_EQ(destroy_count, create_count);
}

TEST_F(loader, load_and_configure_vm_without_set_option)
{
    // Allow empty option and check the VM supporting no options still fails to accept it.
    supported_options[""] = {""};

    setup("path", "sivmc_create", create_vm_barebone);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto vm = sivmc_load_and_configure("path,a=0,b=1", &ec);
    EXPECT_FALSE(vm);
    EXPECT_TRUE(recorded_options.empty());
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_OPTION_NAME);
    EXPECT_STREQ(sivmc_last_error_msg(), "vm_barebone (path) does not support any options");
    EXPECT_EQ(destroy_count, create_count);

    vm = sivmc_load_and_configure("path,", &ec);
    EXPECT_TRUE(vm);
    EXPECT_TRUE(recorded_options.empty());
    EXPECT_EQ(ec, SIVMC_LOADER_SUCCESS);
    EXPECT_FALSE(sivmc_last_error_msg());
    sivmc_destroy(vm);
    EXPECT_EQ(destroy_count, create_count);

    vm = sivmc_load_and_configure("path,,", &ec);
    EXPECT_FALSE(vm);
    EXPECT_TRUE(recorded_options.empty());
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_OPTION_NAME);
    EXPECT_STREQ(sivmc_last_error_msg(), "vm_barebone (path) does not support any options");
    EXPECT_EQ(destroy_count, create_count);
}

TEST_F(loader, load_and_configure_config_too_long)
{
    setup("path", "sivmc_create", create_vm_barebone);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    auto config = std::string{"path,"};
    config.append(10000, 'x');
    auto vm = sivmc_load_and_configure(config.c_str(), &ec);
    EXPECT_FALSE(vm);
    EXPECT_TRUE(recorded_options.empty());
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_ARGUMENT);
    EXPECT_STREQ(sivmc_last_error_msg(),
                 "invalid argument: configuration is too long (maximum allowed length is 4096)");
    EXPECT_EQ(destroy_count, create_count);
}

TEST_F(loader, load_and_configure_error_not_wanted)
{
    setup("path", "sivmc_create", create_vm_with_set_option);

    auto vm = sivmc_load_and_configure("path,f=1", nullptr);
    EXPECT_FALSE(vm);
    ASSERT_EQ(recorded_options.size(), size_t{1});
    EXPECT_EQ(recorded_options[0].first, "f");
    EXPECT_EQ(recorded_options[0].second, "1");
    EXPECT_EQ(destroy_count, create_count);
    EXPECT_STREQ(sivmc_last_error_msg(), "vm_with_set_option (path): unknown option 'f'");
    EXPECT_FALSE(sivmc_last_error_msg());
}

TEST_F(loader, load_and_configure_unknown_set_option_error_code)
{
    // Enable "option name causing unknown error".
    supported_options[option_name_causing_unknown_error] = {""};

    setup("path", "sivmc_create", create_vm_with_set_option);

    sivmc_loader_error_code ec = SIVMC_LOADER_UNSPECIFIED_ERROR;
    const auto config_str = "path," + option_name_causing_unknown_error + "=1";
    auto vm = sivmc_load_and_configure(config_str.c_str(), &ec);
    EXPECT_FALSE(vm);
    ASSERT_EQ(recorded_options.size(), 1u);
    EXPECT_EQ(recorded_options[0].first, option_name_causing_unknown_error);
    EXPECT_EQ(recorded_options[0].second, "1");
    EXPECT_EQ(ec, SIVMC_LOADER_INVALID_OPTION_VALUE);
    EXPECT_EQ(sivmc_last_error_msg(),
              "vm_with_set_option (path): unknown error when setting value '1' for option '" +
                  option_name_causing_unknown_error + "'");
    EXPECT_EQ(destroy_count, create_count);
}
