// SIVMC: Sila VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

use sivmc_declare::sivmc_declare_vm;
use sivmc_vm::ExecutionContext;
use sivmc_vm::ExecutionMessage;
use sivmc_vm::ExecutionResult;
use sivmc_vm::SetOptionError;
use sivmc_vm::SivmcVm;
use std::collections::HashMap;

#[sivmc_declare_vm("Foo VM", "1.42-alpha.gamma.starship")]
pub struct FooVM {
    options: HashMap<String, String>,
}

impl SivmcVm for FooVM {
    fn init() -> Self {
        Self {
            options: Default::default(),
        }
    }

    fn set_option(&mut self, key: &str, value: &str) -> Result<(), SetOptionError> {
        self.options.insert(key.to_string(), value.to_string());

        Ok(())
    }

    fn execute(
        &self,
        _revision: sivmc_sys::sivmc_revision,
        _code: &[u8],
        _message: &ExecutionMessage,
        _context: Option<&mut ExecutionContext>,
    ) -> ExecutionResult {
        ExecutionResult::success(1337, 21, None)
    }
}
