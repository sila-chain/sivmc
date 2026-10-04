// SIVMC: Sila VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

use crate::SivmcVm;

use std::ops::{Deref, DerefMut};

/// Container struct for SIVMC instances and user-defined data.
pub struct SivmcContainer<T>
where
    T: SivmcVm + Sized,
{
    #[allow(dead_code)]
    instance: ::sivmc_sys::sivmc_vm,
    vm: T,
}

impl<T> SivmcContainer<T>
where
    T: SivmcVm + Sized,
{
    /// Basic constructor.
    pub fn new(_instance: ::sivmc_sys::sivmc_vm) -> Box<Self> {
        Box::new(Self {
            instance: _instance,
            vm: T::init(),
        })
    }

    /// Take ownership of the given pointer and return a box.
    ///
    /// # Safety
    /// This function expects a valid instance to be passed.
    pub unsafe fn from_ffi_pointer(instance: *mut ::sivmc_sys::sivmc_vm) -> Box<Self> {
        assert!(!instance.is_null(), "from_ffi_pointer received NULL");
        Box::from_raw(instance as *mut SivmcContainer<T>)
    }

    /// Convert boxed self into an FFI pointer, surrendering ownership of the heap data.
    ///
    /// # Safety
    /// This function will return a valid instance pointer.
    pub unsafe fn into_ffi_pointer(boxed: Box<Self>) -> *mut ::sivmc_sys::sivmc_vm {
        Box::into_raw(boxed) as *mut ::sivmc_sys::sivmc_vm
    }
}

impl<T> Deref for SivmcContainer<T>
where
    T: SivmcVm,
{
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.vm
    }
}

impl<T> DerefMut for SivmcContainer<T>
where
    T: SivmcVm,
{
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.vm
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::types::*;
    use crate::{ExecutionContext, ExecutionMessage, ExecutionResult};

    struct TestVm {}

    impl SivmcVm for TestVm {
        fn init() -> Self {
            TestVm {}
        }
        fn execute(
            &self,
            _revision: sivmc_sys::sivmc_revision,
            _code: &[u8],
            _message: &ExecutionMessage,
            _context: Option<&mut ExecutionContext>,
        ) -> ExecutionResult {
            ExecutionResult::failure()
        }
    }

    unsafe extern "C" fn get_dummy_tx_context(
        _context: *mut sivmc_sys::sivmc_host_context,
    ) -> sivmc_sys::sivmc_tx_context {
        sivmc_sys::sivmc_tx_context {
            tx_gas_price: Uint256::default(),
            tx_origin: Address::default(),
            block_coinbase: Address::default(),
            block_number: 0,
            block_timestamp: 0,
            block_gas_limit: 0,
            block_prev_randao: Uint256::default(),
            chain_id: Uint256::default(),
            block_base_fee: Uint256::default(),
            blob_base_fee: Uint256::default(),
            blob_hashes: std::ptr::null(),
            blob_hashes_count: 0,
            block_slot_number: 0,
        }
    }

    #[test]
    fn container_new() {
        let instance = ::sivmc_sys::sivmc_vm {
            abi_version: ::sivmc_sys::SIVMC_ABI_VERSION as i32,
            name: std::ptr::null(),
            version: std::ptr::null(),
            destroy: None,
            execute: None,
            set_option: None,
        };

        let code = [0u8; 0];

        let message = ::sivmc_sys::sivmc_message {
            kind: ::sivmc_sys::sivmc_call_kind::SIVMC_CALL,
            flags: 0,
            depth: 0,
            gas: 0,
            state_gas: 0,
            recipient: ::sivmc_sys::sivmc_address::default(),
            sender: ::sivmc_sys::sivmc_address::default(),
            input_data: std::ptr::null(),
            input_size: 0,
            value: ::sivmc_sys::sivmc_uint256be::default(),
            code_address: ::sivmc_sys::sivmc_address::default(),
            code: std::ptr::null(),
            code_size: 0,
        };
        let message: ExecutionMessage = (&message).into();

        let host = ::sivmc_sys::sivmc_host_interface {
            account_exists: None,
            get_storage: None,
            set_storage: None,
            get_balance: None,
            get_nonce: None,
            get_code_size: None,
            get_code_hash: None,
            copy_code: None,
            selfdestruct: None,
            call: None,
            get_tx_context: Some(get_dummy_tx_context),
            get_block_hash: None,
            emit_log: None,
            access_account: None,
            access_storage: None,
            get_transient_storage: None,
            set_transient_storage: None,
        };
        let host_context = std::ptr::null_mut();

        let mut context = ExecutionContext::new(&host, host_context);
        let container = SivmcContainer::<TestVm>::new(instance);
        assert_eq!(
            container
                .execute(
                    sivmc_sys::sivmc_revision::SIVMC_PETERSBURG,
                    &code,
                    &message,
                    Some(&mut context)
                )
                .status_code(),
            ::sivmc_sys::sivmc_status_code::SIVMC_FAILURE
        );

        let ptr = unsafe { SivmcContainer::into_ffi_pointer(container) };

        let mut context = ExecutionContext::new(&host, host_context);
        let container = unsafe { SivmcContainer::<TestVm>::from_ffi_pointer(ptr) };
        assert_eq!(
            container
                .execute(
                    sivmc_sys::sivmc_revision::SIVMC_PETERSBURG,
                    &code,
                    &message,
                    Some(&mut context)
                )
                .status_code(),
            ::sivmc_sys::sivmc_status_code::SIVMC_FAILURE
        );
    }
}
