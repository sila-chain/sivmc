# SIVMC VM Tester {#vmtester}

The SIVMC project contains a SIVMC-compatibility testing tool for VM implementations.

The tool is called `sivmc-vmtester` and to include it in the SIVMC build
add `-DSIVMC_TESTING=ON` CMake option to the project configuration step.

Usage is simple as

```sh
sivmc-vmtester [vm]
```

where `[vm]` is a path to a shared library with VM implementation.

For more information check `sivmc-vmtester --help`.
