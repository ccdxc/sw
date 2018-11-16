# Building Delphi Framework

Delphi can be built either in Venice vagrant VM or Nic `make docker/shell` container.

See [here](../../../docs/dev.md) for instructions on how to bringup Venice vagrant VMs.
See [here](../../README.md) for instruction on how to bringup `make docker/shell` container for Nic builds.

### 0. Build the delphi complier (Required only in Venice vagrant VMs)

Delphi complier is built as part of `make ws-tools` build target. In case of `make docker/shell` container, this happens automatically when container is run.

```
cd /import/src/github.com/pensando/sw/
make ws-tools
```

This will build the delphi protobuf plugin which is written in Go language. Delphi compiler
`nic/delphi/compiler/delphic` is a python wrapper script that invokes the `protoc` compiler with proper options to invoke the delphi plugin.

### 1. Build delphi framework and hub

Delphi framework and hub can be built by simply running `make build` in `nic/delphi` directory.

```
cd nic/delphi
make build
```

This will build the delphi SDK libraries, delphi hub binary and the example applications.
Delphi hub binary can be found at `$WORKSPACE/nic/build/x86_64/iris/bin/delphi_hub`. Example application binary is found at `$WORKSPACE/nic/build/x86_64/iris/bin/delphi_example`.

### 2. Unit test delphi framework

Delphi unit tests can be run by invoking `make test` under `nic/delphi/` direcory.

```
cd nic/delphi
make test
```
