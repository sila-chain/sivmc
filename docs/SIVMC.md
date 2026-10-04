# SIVMC – Sila VM Connector API {#mainpage}

**ABI version 19**

The SIVMC is the low-level ABI between Sila Virtual Machines (Sivms) and
Sila Clients. On the Client-side it defines the interface for Sivm implementations
to access Sila environment and state.


# Guides {#guides}

- [Host Implementation Guide](@ref hostguide)
- [VM Implementation Guide](@ref vmguide)


# Versioning {#versioning}

The SIVMC project uses [Semantic Versioning](https://semver.org).
The version format is `MAJOR.MINOR.PATCH`.

The **SIVMC ABI version** is available to VM and Host implementations by
::SIVMC_ABI_VERSION. Every C ABI breaking change increases the ABI version
and requires increasing the _MAJOR_ version number.

The releases with _MINOR_ version change allow adding new API features
and modifying the language bindings API.
Backward incompatible API changes are allowed but should be avoided if possible.

The releases with _PATCH_ should only include bug fixes. Exceptionally,
API changes are allowed when required to fix a broken feature.


# Modules {#modules}

- [SIVMC](@ref SIVMC)
   – the main component that defines API for VMs and Clients (Hosts).
- [SIVMC C++ API](@ref sivmc)
   – the wrappers and bindings for C++.
- [SIVMC Loader](@ref loader)
   – the library for loading VMs implemented as Dynamically Loaded Libraries (DLLs, shared objects).
- [SIVMC Helpers](@ref helpers)
   – a collection of utility functions for easier integration with SIVMC.
- [Sivm Instructions](@ref instructions)
   – the library with collection of metrics for the Sivm instruction set.
- [SIVMC VM Tester](@ref vmtester)
   – the SIVMC-compatibility testing tool for VM implementations.


# Language bindings {#bindings}

## Go

```go
import "github.com/sila-chain/sivmc/bindings/go/sivmc"
```



@addtogroup SIVMC

## Terms

1. **VM** – A Sila Virtual Machine instance/implementation.
2. **Host** – An entity controlling the VM.
   The Host requests code execution and responses to VM queries by callback
   functions. This usually represents a Sila Client.


## Responsibilities

### VM

- Executes the code (obviously).
- Calculates the running gas cost and manages the gas counter except the refund
  counter.
- Controls the call depth, including the exceptional termination of execution
  in case the maximum depth is reached.


### Host

- Provides access to State.
- Creates new accounts (with code being a result of VM execution).
- Handles refunds entirely.
- Manages the set of precompiled contracts and handles execution of messages
  coming to them.
