# SIVMC

[![readme style: standard][readme style standard badge]][standard readme]

> Sila VM Connector API

The SIVMC is the low-level ABI between Sila Virtual Machines (Sivms) and
Sila Clients. On the Client-side it defines the interface for Sivm implementations
to access Sila environment and state.


## Usage

### Documentation

The documentation is in [docs](docs) and is generated from the headers with Doxygen.

### Languages support

| Language                      | Supported Versions   | Supported Compilers          | Feature Support   |
|-------------------------------|----------------------|------------------------------|-------------------|
| **C**                         | C99, C11             | GCC 8+, clang 9+, MSVC 2017+ | Host- and VM-side |
| **C++**                       | C++17                | GCC 8+, clang 9+, MSVC 2017+ | Host- and VM-side |
| **Go** _(bindings)_           | 1.11+ (with modules) |                              | Host-side only    |
| **Rust** _(bindings)_[¹](#n1) | 2018 edition         | 1.47.0 and newer             | VM-side only      |

1. <sup id="n1">↑</sup> Rust support is limited and not complete yet, but it is mostly functional already. Breaking changes are possible at this stage.

### Testing tools

* **sivmc run** ([tools/sivmc]) — executes bytecode in any SIVMC-compatible VM implementation.
* **sivmc-vmtester** ([tools/vmtester]) — can test any Sivm implementation for compatibility with SIVMC.


## Related projects

- [sivmone] — the Sila Virtual Machine implementation using SIVMC.

## Maintainer

[sila-chain]

See also the list of [authors](AUTHORS.md).

## License

[![license badge]][Apache License, Version 2.0]

Licensed under the [Apache License, Version 2.0].


[sila-chain]: https://github.com/sila-chain
[Apache License, Version 2.0]: LICENSE
[sivmone]: https://github.com/sila-chain/sivmone
[standard readme]: https://github.com/RichardLitt/standard-readme
[tools/sivmc]: tools/sivmc
[tools/vmtester]: tools/vmtester

[license badge]: https://img.shields.io/github/license/sila-chain/sivmc.svg?logo=apache
[readme style standard badge]: https://img.shields.io/badge/readme%20style-standard-brightgreen.svg
