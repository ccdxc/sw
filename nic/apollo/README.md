## Coding Style

PDS HAL coding style is documented at https://github.com/pensando/sw/blob/master/nic/README.md

Please follow the coding style strictly and in future these will be enforced
with clang rules as much as possible.

## Tracing

Please don't add traces that aren't useful (e.g. function enter/exit traces).
The traces will already have function name, line# in them and no additional
information is needed. Instead of traces, preferred approach is to add counters
and add pdsctl show CLI.

## PDS HAL directory layout

nic/apollo/agent/ - agent code, sub directories here are self-explanatory
nic/apollo/core/ - core services of HAL
nic/apollo/framework/ - API engine/framework
nic/apollo/nicmgr/ - nicmgr thread specific code
nic/apollo/pciemgr/ - pciemgr thread specific code
nic/apollo/p4/ & nic/apollo/asm/ - datapath code
nic/apollo/vpp/ - VPP infra and plugins
nic/apollo/test/ - apollo gtests
nic/apollo/api/ - apollo API implementation
nic/apollo/api/impl/apollo/ - apollo pipeline dependent code
nic/apollo/api/impl/artemis/ - artemis pipeline dependent code
nic/apollo/api/impl/capri/ - capri specific code

## PDS HAL and Agent

There is a clear demarkation between HAL library and Agent. PDS HAL library
must not have any protobuf code (no protobuf generated files must be included
in HAL code). Agent terminates the protobuf calls and invokes HAL library APIs.

## How to define new API

All API names and publicly exposed data structures must have pds_ as prefix,
please don't use any other prefix for APis to be consumed by (Pensando or
non-Pensando) agents.
