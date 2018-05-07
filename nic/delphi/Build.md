# Building Delphi Framework

Delphi framework can be built using `bazel` build tool. Delphi can be built either in Venice vagrant VM or Nic `make shell` container.

See [here](../../docs/dev.md) for instructions on how to bringup Venice vagrant VMs. Note: running `PENS_NODES=1 make dev` will bringup a single VM for development environment.
See [here](../README.md) for instruction on how to bringup `make shell` container for Nic builds.

### 0. Build the delphi complier (Required only in Venice vagrant VMs)

Delphi complier is built as part of `make ws-tools` build target. In case of `make shell` container, this happens automatically when container is run.

```
cd /import/src/github.com/pensando/sw/
make ws-tools
```

This will build the delphi protobuf plugin which is written in Go language. Delphi compiler
`nic/delphi/compiler/delphic` is a python wrapper script that invokes the `protoc` compiler with proper options to invoke the delphi plugin.

### 1. Build delphi framework and hub

Delphi framework and hub can be built by simply running `bazel build`.

```
cd nic
make pull-assets
bazel build //nic/delphi/...
```

This will build the delphi SDK libraries, delphi hub binary and the example applications.
Delphi hub binary can be found at `$WORKSPACE/bazel-bin/nic/delphi/hub/delphi_hub`. Example application binary is found at `$WORKSPACE/bazel-bin/nic/delphi/example/example`.

### 2. Unit test delphi framework

Delphi unit tests can be run by invoking `delphi test`.

```
bazel test //nic/delphi/... --jobs 1 --cache_test_results=no --test_output=all
```
