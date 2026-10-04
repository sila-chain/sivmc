// SIVMC: Sila VM Connector API.
// Copyright 2018 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.

package sivmc

/*
#cgo CFLAGS: -I${SRCDIR}/../../../include -Wall -Wextra
#cgo !windows LDFLAGS: -ldl

#include <sivmc/sivmc.h>
#include <sivmc/helpers.h>
#include <sivmc/loader.h>

#include <stdlib.h>
#include <string.h>

static inline enum sivmc_set_option_result set_option(struct sivmc_vm* vm, char* name, char* value)
{
	enum sivmc_set_option_result ret = sivmc_set_option(vm, name, value);
	free(name);
	free(value);
	return ret;
}

extern const struct sivmc_host_interface sivmc_go_host;

static struct sivmc_result execute_wrapper(struct sivmc_vm* vm,
	uintptr_t context_index, enum sivmc_revision rev,
	enum sivmc_call_kind kind, uint32_t flags, int32_t depth, int64_t gas, int64_t state_gas,
	const sivmc_address* recipient, const sivmc_address* sender,
	const uint8_t* input_data, size_t input_size, const sivmc_uint256be* value,
	const uint8_t* code, size_t code_size)
{
	struct sivmc_message msg = {
		kind,
		flags,
		depth,
		gas,
		state_gas,
		*recipient,
		*sender,
		input_data,
		input_size,
		*value,
		{{0}}, // code_address: not required for execution
		0,     // code
		0,     // code_size
	};

	struct sivmc_host_context* context = (struct sivmc_host_context*)context_index;
	return sivmc_execute(vm, &sivmc_go_host, context, rev, &msg, code, code_size);
}
*/
import "C"

import (
	"fmt"
	"sync"
	"unsafe"
)

// Hash represents the 32 bytes of arbitrary data (e.g. the result of Keccak256
// hash). It occasionally is used to represent 256-bit unsigned integer values
// stored in big-endian byte order.
type Hash [32]byte

// Address represents the 160-bit (20 bytes) address of a Sila account.
type Address [20]byte

// Static asserts.
const (
	// The size of sivmc_bytes32 equals the size of Hash.
	_ = uint(len(Hash{}) - C.sizeof_sivmc_bytes32)
	_ = uint(C.sizeof_sivmc_bytes32 - len(Hash{}))

	// The size of sivmc_address equals the size of Address.
	_ = uint(len(Address{}) - C.sizeof_sivmc_address)
	_ = uint(C.sizeof_sivmc_address - len(Address{}))
)

type Error int32

func (err Error) IsInternalError() bool {
	return err < 0
}

func (err Error) Error() string {
	return C.GoString(C.sivmc_status_code_to_string(C.enum_sivmc_status_code(err)))
}

const (
	Failure = Error(C.SIVMC_FAILURE)
	Revert  = Error(C.SIVMC_REVERT)
)

type Revision int32

const (
	Frontier             Revision = C.SIVMC_FRONTIER
	Homestead            Revision = C.SIVMC_HOMESTEAD
	TangerineWhistle     Revision = C.SIVMC_TANGERINE_WHISTLE
	SpuriousDragon       Revision = C.SIVMC_SPURIOUS_DRAGON
	Byzantium            Revision = C.SIVMC_BYZANTIUM
	Petersburg           Revision = C.SIVMC_PETERSBURG
	Istanbul             Revision = C.SIVMC_ISTANBUL
	Berlin               Revision = C.SIVMC_BERLIN
	London               Revision = C.SIVMC_LONDON
	Paris                Revision = C.SIVMC_PARIS
	Shanghai             Revision = C.SIVMC_SHANGHAI
	Cancun               Revision = C.SIVMC_CANCUN
	Prague               Revision = C.SIVMC_PRAGUE
	Osaka                Revision = C.SIVMC_OSAKA
	Amsterdam            Revision = C.SIVMC_AMSTERDAM
	Experimental         Revision = C.SIVMC_EXPERIMENTAL
	MaxRevision          Revision = C.SIVMC_MAX_REVISION
	LatestStableRevision Revision = C.SIVMC_LATEST_STABLE_REVISION
)

type VM struct {
	handle *C.struct_sivmc_vm
}

func Load(filename string) (vm *VM, err error) {
	cfilename := C.CString(filename)
	loaderErr := C.enum_sivmc_loader_error_code(C.SIVMC_LOADER_UNSPECIFIED_ERROR)
	handle := C.sivmc_load_and_create(cfilename, &loaderErr)
	C.free(unsafe.Pointer(cfilename))

	if loaderErr == C.SIVMC_LOADER_SUCCESS {
		vm = &VM{handle}
	} else {
		errMsg := C.sivmc_last_error_msg()
		if errMsg != nil {
			err = fmt.Errorf("SIVMC loading error: %s", C.GoString(errMsg))
		} else {
			err = fmt.Errorf("SIVMC loading error %d", int(loaderErr))
		}
	}

	return vm, err
}

func LoadAndConfigure(config string) (vm *VM, err error) {
	cconfig := C.CString(config)
	loaderErr := C.enum_sivmc_loader_error_code(C.SIVMC_LOADER_UNSPECIFIED_ERROR)
	handle := C.sivmc_load_and_configure(cconfig, &loaderErr)
	C.free(unsafe.Pointer(cconfig))

	if loaderErr == C.SIVMC_LOADER_SUCCESS {
		vm = &VM{handle}
	} else {
		errMsg := C.sivmc_last_error_msg()
		if errMsg != nil {
			err = fmt.Errorf("SIVMC loading error: %s", C.GoString(errMsg))
		} else {
			err = fmt.Errorf("SIVMC loading error %d", int(loaderErr))
		}
	}

	return vm, err
}

func (vm *VM) Destroy() {
	C.sivmc_destroy(vm.handle)
}

func (vm *VM) Name() string {
	// TODO: consider using C.sivmc_vm_name(vm.handle)
	return C.GoString(vm.handle.name)
}

func (vm *VM) Version() string {
	// TODO: consider using C.sivmc_vm_version(vm.handle)
	return C.GoString(vm.handle.version)
}

func (vm *VM) SetOption(name string, value string) (err error) {

	r := C.set_option(vm.handle, C.CString(name), C.CString(value))
	switch r {
	case C.SIVMC_SET_OPTION_INVALID_NAME:
		err = fmt.Errorf("sivmc: option '%s' not accepted", name)
	case C.SIVMC_SET_OPTION_INVALID_VALUE:
		err = fmt.Errorf("sivmc: option '%s' has invalid value", name)
	case C.SIVMC_SET_OPTION_SUCCESS:
	}
	return err
}

// StateGas contains the state-gas counters of an execution (SIP-8037).
type StateGas struct {
	Left    int64
	Spilled int64
}

type Result struct {
	Output    []byte
	GasLeft   int64
	GasRefund int64
	StateGas  StateGas
}

func (vm *VM) Execute(ctx HostContext, rev Revision,
	kind CallKind, static bool, delegated bool, depth int, gas int64, stateGas int64,
	recipient Address, sender Address, input []byte, value Hash,
	code []byte) (res Result, err error) {

	flags := C.uint32_t(0)
	if static {
		flags |= C.SIVMC_STATIC
	}
	if delegated {
		flags |= C.SIVMC_DELEGATED
	}

	ctxId := addHostContext(ctx)
	// FIXME: Clarify passing by pointer vs passing by value.
	sivmcRecipient := sivmcAddress(recipient)
	sivmcSender := sivmcAddress(sender)
	sivmcValue := sivmcBytes32(value)
	result := C.execute_wrapper(vm.handle, C.uintptr_t(ctxId), uint32(rev),
		C.enum_sivmc_call_kind(kind), flags, C.int32_t(depth), C.int64_t(gas), C.int64_t(stateGas),
		&sivmcRecipient, &sivmcSender, bytesPtr(input), C.size_t(len(input)), &sivmcValue,
		bytesPtr(code), C.size_t(len(code)))
	removeHostContext(ctxId)

	res.Output = C.GoBytes(unsafe.Pointer(result.output_data), C.int(result.output_size))
	res.GasLeft = int64(result.gas_left)
	res.GasRefund = int64(result.gas_refund)
	res.StateGas = StateGas{int64(result.state_gas.left), int64(result.state_gas.spilled)}
	if result.status_code != C.SIVMC_SUCCESS {
		err = Error(result.status_code)
	}

	if result.release != nil {
		C.sivmc_release_result(&result)
	}

	return res, err
}

var (
	hostContextCounter uintptr
	hostContextMap     = map[uintptr]HostContext{}
	hostContextMapMu   sync.Mutex
)

func addHostContext(ctx HostContext) uintptr {
	hostContextMapMu.Lock()
	id := hostContextCounter
	hostContextCounter++
	hostContextMap[id] = ctx
	hostContextMapMu.Unlock()
	return id
}

func removeHostContext(id uintptr) {
	hostContextMapMu.Lock()
	delete(hostContextMap, id)
	hostContextMapMu.Unlock()
}

func getHostContext(idx uintptr) HostContext {
	hostContextMapMu.Lock()
	ctx := hostContextMap[idx]
	hostContextMapMu.Unlock()
	return ctx
}

func sivmcBytes32(in Hash) C.sivmc_bytes32 {
	out := C.sivmc_bytes32{}
	for i := 0; i < len(in); i++ {
		out.bytes[i] = C.uint8_t(in[i])
	}
	return out
}

func sivmcAddress(address Address) C.sivmc_address {
	r := C.sivmc_address{}
	for i := 0; i < len(address); i++ {
		r.bytes[i] = C.uint8_t(address[i])
	}
	return r
}

func bytesPtr(bytes []byte) *C.uint8_t {
	if len(bytes) == 0 {
		return nil
	}
	return (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
}
