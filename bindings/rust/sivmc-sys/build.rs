// SIVMC: Sila VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn gen_bindings() {
    let bindings = bindgen::Builder::default()
        .header("sivmc.h")
        // C23 gives sivmc_access_status the same bool underlying type as in C++
        .clang_arg("-std=c2x")
        .generate_comments(true)
        // do not generate an empty enum for SIVMC_ABI_VERSION
        .constified_enum("")
        // generate Rust enums for each sivmc enum
        .rustified_enum(".*")
        // force deriving the Hash trait on basic types (address, bytes32)
        .derive_hash(true)
        // force deriving the PratialEq trait on basic types (address, bytes32)
        .derive_partialeq(true)
        .blocklist_type("sivmc_host_context")
        .allowlist_type("sivmc_.*")
        .allowlist_function("sivmc_.*")
        .allowlist_var("SIVMC_ABI_VERSION")
        // TODO: consider removing this
        .size_t_is_usize(true)
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}

fn main() {
    gen_bindings();
}
