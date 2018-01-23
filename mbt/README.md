Introduction:
The Model Based Tester is a tool which generates random configurations, and tests the resiliency of SDK applications( example:HAL ) based on these configurations. It can be run in 2 modes
1) DOL Mode
    To use DOL mode, append --mbt option when running the run.py script. This will start model based tester as a separate process, which intercepts the GRPC communication between the SDK application and DOL. MBT then proceeds to modify the config from DOL by performing any of the CRUD operations, before restoring the original DOL config and running the packet tests.
Example Usage:
    Currently we run the networking DOL test from the nic/ folder as "./run.py --topo up2up --feature networking". To run the same test with MBT, we just need to append --mbt to the argument i.e. "./run.py --topo up2up --feature networking --mbt".
2) Independent mode.
    MBT generates random configuration and programs them on SDK application, and makes sure that the SDK application reacts with appropriate response codes.
Example Usage:
    In the nic/ folder, run "./run.py --mbt" to start Model Based Tester in independent mode.

Using the Model Based tester for your object:
To enable model based tester for an object, 2 files have to be created.
1) The spec file located in cfg/specs, which lists the object specific information
2) The callback file located in cfg/callbacks, which can be used to enforce any restrictions while sending a message.

1) The spec file located in cfg/specs, which lists the object specific
   information
2) The callback file located in cfg/callbacks, which can be used to enforce any
   restrictions while sending a message.

To specify sequences of operation when running in independent mode, add them in
   mbt_test/specs. Take a look at mbt_test/specs/create_delete.spec as a reference.

Please note that currently, the sequence of operations performed for the DOL
mode have been hardcoded. This will be updated in the future.

Currently, model based tester supports the following objects
1) SecurityGroup
2) SecurityProfile
3) Interface
4) LIF
5) Vrf
6) L2Segment
7) Endpoint
8) Network
9) SecurityGroupPolicy

Tags Supported:
Currently, model based supports the following tags
1) Range tag: This tag is used to define ranges of possible values for integers
Ex : uint32 ip_normalize_ttl = 25 [(gogoproto.moretags) = "range:0-255"];
2) Key Handle Tag : This tag is used to indicate this field is being used to refer to a config object inside the SDK application.
Ex : kh.VrfKeyHandle key_or_handle = 2 [(gogoproto.moretags) = "venice:\"key\""];
3) Reference tag : This is used to indicate that this field is a reference to a pre existing object inside the SDK application. 
Ex : kh.LifKeyHandle lif_key_or_handle = 2 [(gogoproto.moretags) = "ref"];
