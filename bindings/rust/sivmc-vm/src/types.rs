use sivmc_sys as ffi;

/// SIVMC address
pub type Address = ffi::sivmc_address;

/// SIVMC 32 bytes value (used for hashes)
pub type Bytes32 = ffi::sivmc_bytes32;

/// SIVMC big-endian 256-bit integer
pub type Uint256 = ffi::sivmc_uint256be;

/// SIVMC call kind.
pub type MessageKind = ffi::sivmc_call_kind;

/// SIVMC message (call) flags.
pub type MessageFlags = ffi::sivmc_flags;

/// SIVMC status code.
pub type StatusCode = ffi::sivmc_status_code;

/// SIVMC access status.
pub type AccessStatus = ffi::sivmc_access_status;

/// SIVMC storage status.
pub type StorageStatus = ffi::sivmc_storage_status;

/// SIVMC state-gas counters (SIP-8037).
pub type StateGas = ffi::sivmc_state_gas;

/// SIVMC VM revision.
pub type Revision = ffi::sivmc_revision;

#[cfg(test)]
mod tests {
    use super::*;

    // These tests check for Default, PartialEq and Clone traits.
    #[test]
    fn address_smoke_test() {
        let a = ffi::sivmc_address::default();
        let b = Address::default();
        assert_eq!(a.clone(), b.clone());
    }

    #[test]
    fn bytes32_smoke_test() {
        let a = ffi::sivmc_bytes32::default();
        let b = Bytes32::default();
        assert_eq!(a.clone(), b.clone());
    }

    #[test]
    fn uint26be_smoke_test() {
        let a = ffi::sivmc_uint256be::default();
        let b = Uint256::default();
        assert_eq!(a.clone(), b.clone());
    }

    #[test]
    fn message_kind() {
        assert_eq!(MessageKind::SIVMC_CALL, ffi::sivmc_call_kind::SIVMC_CALL);
        assert_eq!(
            MessageKind::SIVMC_CALLCODE,
            ffi::sivmc_call_kind::SIVMC_CALLCODE
        );
        assert_eq!(
            MessageKind::SIVMC_DELEGATECALL,
            ffi::sivmc_call_kind::SIVMC_DELEGATECALL
        );
        assert_eq!(
            MessageKind::SIVMC_CREATE,
            ffi::sivmc_call_kind::SIVMC_CREATE
        );
        assert_eq!(
            MessageKind::SIVMC_CREATE2,
            ffi::sivmc_call_kind::SIVMC_CREATE2
        );
    }

    #[test]
    fn message_flags() {
        assert_eq!(MessageFlags::SIVMC_STATIC, ffi::sivmc_flags::SIVMC_STATIC);
    }

    #[test]
    fn status_code() {
        assert_eq!(
            StatusCode::SIVMC_SUCCESS,
            ffi::sivmc_status_code::SIVMC_SUCCESS
        );
        assert_eq!(
            StatusCode::SIVMC_FAILURE,
            ffi::sivmc_status_code::SIVMC_FAILURE
        );
    }

    #[test]
    fn access_status() {
        assert_eq!(
            AccessStatus::SIVMC_ACCESS_COLD,
            ffi::sivmc_access_status::SIVMC_ACCESS_COLD
        );
        assert_eq!(
            AccessStatus::SIVMC_ACCESS_WARM,
            ffi::sivmc_access_status::SIVMC_ACCESS_WARM
        );
    }

    #[test]
    fn storage_status() {
        assert_eq!(
            StorageStatus::SIVMC_STORAGE_ASSIGNED,
            ffi::sivmc_storage_status::SIVMC_STORAGE_ASSIGNED
        );
        assert_eq!(
            StorageStatus::SIVMC_STORAGE_MODIFIED,
            ffi::sivmc_storage_status::SIVMC_STORAGE_MODIFIED
        );
    }

    #[test]
    fn revision() {
        assert_eq!(
            Revision::SIVMC_FRONTIER,
            ffi::sivmc_revision::SIVMC_FRONTIER
        );
        assert_eq!(
            Revision::SIVMC_SILA_ISTANBUL,
            ffi::sivmc_revision::SIVMC_SILA_ISTANBUL
        );
    }
}
