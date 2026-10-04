// SIVMC: Sila VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

package sivmc

/*
#cgo CFLAGS: -I${SRCDIR}/../../../include -Wall -Wextra -Wno-unused-parameter

#include <sivmc/sivmc.h>
#include <sivmc/helpers.h>

*/
import "C"
import (
	"unsafe"
)

type CallKind int

const (
	Call         CallKind = C.SIVMC_CALL
	DelegateCall CallKind = C.SIVMC_DELEGATECALL
	CallCode     CallKind = C.SIVMC_CALLCODE
	Create       CallKind = C.SIVMC_CREATE
	Create2      CallKind = C.SIVMC_CREATE2
)

type AccessStatus int

const (
	ColdAccess AccessStatus = C.SIVMC_ACCESS_COLD
	WarmAccess AccessStatus = C.SIVMC_ACCESS_WARM
)

type StorageStatus int

const (
	StorageAssigned         StorageStatus = C.SIVMC_STORAGE_ASSIGNED
	StorageAdded            StorageStatus = C.SIVMC_STORAGE_ADDED
	StorageDeleted          StorageStatus = C.SIVMC_STORAGE_DELETED
	StorageModified         StorageStatus = C.SIVMC_STORAGE_MODIFIED
	StorageDeletedAdded     StorageStatus = C.SIVMC_STORAGE_DELETED_ADDED
	StorageModifiedDeleted  StorageStatus = C.SIVMC_STORAGE_MODIFIED_DELETED
	StorageDeletedRestored  StorageStatus = C.SIVMC_STORAGE_DELETED_RESTORED
	StorageAddedDeleted     StorageStatus = C.SIVMC_STORAGE_ADDED_DELETED
	StorageModifiedRestored StorageStatus = C.SIVMC_STORAGE_MODIFIED_RESTORED
)

func goAddress(in C.sivmc_address) Address {
	out := Address{}
	for i := 0; i < len(out); i++ {
		out[i] = byte(in.bytes[i])
	}
	return out
}

func goHash(in C.sivmc_bytes32) Hash {
	out := Hash{}
	for i := 0; i < len(out); i++ {
		out[i] = byte(in.bytes[i])
	}
	return out
}

func goByteSlice(data *C.uint8_t, size C.size_t) []byte {
	if size == 0 {
		return []byte{}
	}
	return (*[1 << 30]byte)(unsafe.Pointer(data))[:size:size]
}

// TxContext contains information about current transaction and block.
type TxContext struct {
	GasPrice    Hash
	Origin      Address
	Coinbase    Address
	Number      int64
	Timestamp   int64
	GasLimit    int64
	PrevRandao  Hash
	ChainID     Hash
	BaseFee     Hash
	BlobBaseFee Hash
	SlotNumber  uint64
}

type HostContext interface {
	AccountExists(addr Address) bool
	GetStorage(addr Address, key Hash) Hash
	SetStorage(addr Address, key Hash, value Hash) StorageStatus
	GetBalance(addr Address) Hash
	GetNonce(addr Address) uint64
	GetCodeSize(addr Address) int
	GetCodeHash(addr Address) Hash
	GetCode(addr Address) []byte
	Selfdestruct(addr Address, beneficiary Address) bool
	GetTxContext() TxContext
	GetBlockHash(number int64) Hash
	EmitLog(addr Address, topics []Hash, data []byte)
	Call(kind CallKind,
		recipient Address, sender Address, value Hash, input []byte, gas int64, stateGas int64,
		depth int, static bool, codeAddress Address) (output []byte, gasLeft int64, gasRefund int64,
		resultStateGas StateGas, err error)
	AccessAccount(addr Address) AccessStatus
	AccessStorage(addr Address, key Hash) AccessStatus
	GetTransientStorage(addr Address, key Hash) Hash
	SetTransientStorage(addr Address, key Hash, value Hash)
}

//export accountExists
func accountExists(pCtx unsafe.Pointer, pAddr *C.sivmc_address) C.bool {
	ctx := getHostContext(uintptr(pCtx))
	return C.bool(ctx.AccountExists(goAddress(*pAddr)))
}

//export getStorage
func getStorage(pCtx unsafe.Pointer, pAddr *C.struct_sivmc_address, pKey *C.sivmc_bytes32) C.sivmc_bytes32 {
	ctx := getHostContext(uintptr(pCtx))
	return sivmcBytes32(ctx.GetStorage(goAddress(*pAddr), goHash(*pKey)))
}

//export setStorage
func setStorage(pCtx unsafe.Pointer, pAddr *C.sivmc_address, pKey *C.sivmc_bytes32, pVal *C.sivmc_bytes32) C.enum_sivmc_storage_status {
	ctx := getHostContext(uintptr(pCtx))
	return C.enum_sivmc_storage_status(ctx.SetStorage(goAddress(*pAddr), goHash(*pKey), goHash(*pVal)))
}

//export getBalance
func getBalance(pCtx unsafe.Pointer, pAddr *C.sivmc_address) C.sivmc_uint256be {
	ctx := getHostContext(uintptr(pCtx))
	return sivmcBytes32(ctx.GetBalance(goAddress(*pAddr)))
}

//export getNonce
func getNonce(pCtx unsafe.Pointer, pAddr *C.sivmc_address) C.uint64_t {
	ctx := getHostContext(uintptr(pCtx))
	return C.uint64_t(ctx.GetNonce(goAddress(*pAddr)))
}

//export getCodeSize
func getCodeSize(pCtx unsafe.Pointer, pAddr *C.sivmc_address) C.size_t {
	ctx := getHostContext(uintptr(pCtx))
	return C.size_t(ctx.GetCodeSize(goAddress(*pAddr)))
}

//export getCodeHash
func getCodeHash(pCtx unsafe.Pointer, pAddr *C.sivmc_address) C.sivmc_bytes32 {
	ctx := getHostContext(uintptr(pCtx))
	return sivmcBytes32(ctx.GetCodeHash(goAddress(*pAddr)))
}

//export copyCode
func copyCode(pCtx unsafe.Pointer, pAddr *C.sivmc_address, offset C.size_t, p *C.uint8_t, size C.size_t) C.size_t {
	ctx := getHostContext(uintptr(pCtx))
	code := ctx.GetCode(goAddress(*pAddr))
	length := C.size_t(len(code))

	if offset >= length {
		return 0
	}

	toCopy := length - offset
	if toCopy > size {
		toCopy = size
	}

	out := goByteSlice(p, size)
	copy(out, code[offset:])
	return toCopy
}

//export selfdestruct
func selfdestruct(pCtx unsafe.Pointer, pAddr *C.sivmc_address, pBeneficiary *C.sivmc_address) C.bool {
	ctx := getHostContext(uintptr(pCtx))
	return C.bool(ctx.Selfdestruct(goAddress(*pAddr), goAddress(*pBeneficiary)))
}

//export getTxContext
func getTxContext(pCtx unsafe.Pointer) C.struct_sivmc_tx_context {
	ctx := getHostContext(uintptr(pCtx))

	txContext := ctx.GetTxContext()

	return C.struct_sivmc_tx_context{
		sivmcBytes32(txContext.GasPrice),
		sivmcAddress(txContext.Origin),
		sivmcAddress(txContext.Coinbase),
		C.int64_t(txContext.Number),
		C.int64_t(txContext.Timestamp),
		C.int64_t(txContext.GasLimit),
		sivmcBytes32(txContext.PrevRandao),
		sivmcBytes32(txContext.ChainID),
		sivmcBytes32(txContext.BaseFee),
		sivmcBytes32(txContext.BlobBaseFee),
		nil, // TODO: Add support for blob hashes.
		0,
		C.uint64_t(txContext.SlotNumber),
	}
}

//export getBlockHash
func getBlockHash(pCtx unsafe.Pointer, number int64) C.sivmc_bytes32 {
	ctx := getHostContext(uintptr(pCtx))
	return sivmcBytes32(ctx.GetBlockHash(number))
}

//export emitLog
func emitLog(pCtx unsafe.Pointer, pAddr *C.sivmc_address, pData unsafe.Pointer, dataSize C.size_t, pTopics unsafe.Pointer, topicsCount C.size_t) {
	ctx := getHostContext(uintptr(pCtx))

	// FIXME: Optimize memory copy
	data := C.GoBytes(pData, C.int(dataSize))
	tData := C.GoBytes(pTopics, C.int(topicsCount*32))

	nTopics := int(topicsCount)
	topics := make([]Hash, nTopics)
	for i := 0; i < nTopics; i++ {
		copy(topics[i][:], tData[i*32:(i+1)*32])
	}

	ctx.EmitLog(goAddress(*pAddr), topics, data)
}

//export call
func call(pCtx unsafe.Pointer, msg *C.struct_sivmc_message) C.struct_sivmc_result {
	ctx := getHostContext(uintptr(pCtx))

	kind := CallKind(msg.kind)
	output, gasLeft, gasRefund, stateGas, err := ctx.Call(kind, goAddress(msg.recipient), goAddress(msg.sender), goHash(msg.value),
		goByteSlice(msg.input_data, msg.input_size), int64(msg.gas), int64(msg.state_gas), int(msg.depth), msg.flags != 0,
		goAddress(msg.code_address))

	statusCode := C.enum_sivmc_status_code(0)
	if err != nil {
		statusCode = C.enum_sivmc_status_code(err.(Error))
	}

	outputData := (*C.uint8_t)(nil)
	if len(output) > 0 {
		outputData = (*C.uint8_t)(&output[0])
	}

	result := C.sivmc_make_result(statusCode, C.int64_t(gasLeft), C.int64_t(gasRefund), outputData, C.size_t(len(output)))
	result.state_gas.left = C.int64_t(stateGas.Left)
	result.state_gas.spilled = C.int64_t(stateGas.Spilled)
	return result
}

//export accessAccount
func accessAccount(pCtx unsafe.Pointer, pAddr *C.sivmc_address) C.enum_sivmc_access_status {
	ctx := getHostContext(uintptr(pCtx))
	return C.enum_sivmc_access_status(ctx.AccessAccount(goAddress(*pAddr)))
}

//export accessStorage
func accessStorage(pCtx unsafe.Pointer, pAddr *C.sivmc_address, pKey *C.sivmc_bytes32) C.enum_sivmc_access_status {
	ctx := getHostContext(uintptr(pCtx))
	return C.enum_sivmc_access_status(ctx.AccessStorage(goAddress(*pAddr), goHash(*pKey)))
}

//export getTransientStorage
func getTransientStorage(pCtx unsafe.Pointer, pAddr *C.struct_sivmc_address, pKey *C.sivmc_bytes32) C.sivmc_bytes32 {
	ctx := getHostContext(uintptr(pCtx))
	return sivmcBytes32(ctx.GetTransientStorage(goAddress(*pAddr), goHash(*pKey)))
}

//export setTransientStorage
func setTransientStorage(pCtx unsafe.Pointer, pAddr *C.sivmc_address, pKey *C.sivmc_bytes32, pVal *C.sivmc_bytes32) {
	ctx := getHostContext(uintptr(pCtx))
	ctx.SetTransientStorage(goAddress(*pAddr), goHash(*pKey), goHash(*pVal))
}
