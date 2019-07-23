## Coding Style

PDS HAL coding style is documented at https://github.com/pensando/sw/blob/master/nic/README.md

Please follow the coding style strictly and in future these will be enforced
with clang rules as much as possible.

## Tracing

* Please don't add traces that aren't useful (e.g. function enter/exit traces)
  in production system.
* All trace statements already have function name, line# in them thats added by
  the trace infra.
* There is pdsctl CLI available to enable traces when needed, hence by default
  please PDS_TRACE_VERBOSE macro only. If and if you need more information in
  production system, please use PDS_TRACE_DEBUG macro.
* Instead of traces, preferred approach is to add counters and add pdsctl show
  CLIs to dump your counters.

## PDS HAL directory layout

* nic/apollo/agent/ - agent code, sub directories here are self-explanatory
* nic/apollo/core/ - core services of HAL
* nic/apollo/framework/ - API engine/framework
* nic/apollo/nicmgr/ - nicmgr thread specific code
* nic/apollo/pciemgr/ - pciemgr thread specific code
* nic/apollo/p4/ & nic/apollo/asm/ - datapath code
* nic/apollo/vpp/ - VPP infra and plugins
* nic/apollo/test/ - apollo gtests
* nic/apollo/api/include/ - PDS HAL (i.e., apollo HAL) public header files
* nic/apollo/api/ - apollo API implementation
* nic/apollo/api/impl/apollo/ - apollo pipeline dependent code
* nic/apollo/api/impl/artemis/ - artemis pipeline dependent code
* nic/apollo/api/impl/capri/ - capri specific code

## PDS HAL and Agent

There is a clear demarcation between HAL library and Agent. PDS HAL library
must not have any protobuf code (no protobuf generated files must be included
in HAL code). Agent terminates the protobuf calls and invokes HAL library APIs
and HAL APIs are implemented in C/C++. PDS HAL is an application built on top
of SDK libraries.

Apollo HAL/agent code MUST NOT include any headers from iris code base and MUST
NOT link to iris libraries. Any code that needs to be shared across pipelines
MUST first be migrated to nic/sdk and then all pipelines can then link to SDK.

## SDK (nic/sdk)

* SDK code base (nic/sdk) MUST NOT include any headers outside SDK code base and
  sdk must be compileable without any external dependencies (sdk jobd job will
  ensure this). Please don't add symlinks or any shortcuts to get around it.

* The same applies to p4 and asm code, if there are some programs that are truly
  common across pipelines, they can be moved to sdk so these common p4/asm
  libraries can be re-used.

* SDK code base must not have pipeline specific includes or #ifdef statements
  as workaround - these are hacks and must be dealt with in the right place.
  It is best not to move such code to SDK until and unless the root cause
  is addressed.

## How to define new API

* All API names and publicly exposed data structures must have pds_ as prefix,
please don't use any other prefix for APis to be consumed by (Pensando or
non-Pensando) agents.

* All user facing header files (inclduing Pensando PDS Agent) are located in
  nic/apollo/api/include/. If you need to define new API or object, please
  see if related header file already exists or not and reuse if there is one.
  Usually new object warrants a new header file to keep it clean.

* All the objects provide CRUD APIs in the following form:

 ** pds_xxx_create()
 ** pds_xxx_read()
 ** pds_xxx_update()
 ** pds_xxx_delete()

  Some objects can provide subset of this based on what is allowed and what is
  not, however, please don't provide any more APIs outside these four CRUD APIs.

Once you define your objects and corresponding CRUD APIs in pds_xxx.hpp, next
step is provide implementation in nic/apollo/api/ directory. For providing
PI (Pipeline Independent) implementation, the following files must be defined:
  * <obj>_api.cc ->
  * <obj>.hpp -> this is the PI class instance corresponding to the object
  * <obj>.cc ->
  * <obj>_state.hpp ->
  * <obj>_state.cc ->

## Google tests (gtests)

## DOL tests

## IOTA tests

## CLIs

## p4ctl CLIs

## Memory
