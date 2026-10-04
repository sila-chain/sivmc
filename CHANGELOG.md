# Changelog

Documentation of all notable changes to the **SIVMC** project.

The format is based on [Keep a Changelog],
and this project adheres to [Semantic Versioning].

## [Unreleased]

SIVMC, the Sila VM Connector API, starts here.

### Changed

- The project, its headers (`include/sivmc`), libraries, CMake package and targets,
  tools, Go module (`github.com/sila-chain/sivmc/v12`) and Rust crates are named SIVMC.
- The API continues at ABI version 19: the host interface has `get_nonce()`, messages
  and results carry state gas, the transaction context carries the slot number,
  and the capabilities, the CREATE2 salt, the result optional storage and the
  Constantinople revision are gone.
- Improvement proposals are referenced as SIPs.


[Unreleased]: https://github.com/sila-chain/sivmc/commits/master
[Keep a Changelog]: https://keepachangelog.com/en/1.1.0/
[Semantic Versioning]: https://semver.org
