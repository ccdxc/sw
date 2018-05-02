#IPC

This IPC is intended for use cases that require non-blocking behavior.
FTE pushing firewall logs is such a use case. For simplicity, a separate
IPC pipe is used for each client thread. Shared memory is the underlying
transport mechanism, with protobuf as the language independent serialization.

There are no mallocs involved. The queue has sufficient storage to absorb
bursts. But if the queue fills up, newer entries would be dropped.

Currently, this is a one-way IPC from FTE to the agent. The sender is
coded in C++ and the receiver in go.

##Layout
Each IPC pipe has the following layout.

base_addr->[tx-control(32b)]
           [rx-control(32b)]
           [tx-count  (64b)]
           [               ]
           [err-count (64b)]
           [               ]
           [write-indx(32b)]
           [read-index(32b)]
           [---------------]
           [   {header}    ]
           [   64 byte     ]
           [   buffers     ]
           [               ]
           ................
           ................
           [   {header}    ]
           [   64 byte     ]
           [   buffers     ]
           [               ]
           ................
           ................
           [---------------]

##Usage

On the send side, #include "nic/utils/agent_api/agent_api.hpp".
Initialize ipc_logger::init() static method. Use ipc_logger::factory()
method to allocate an ipc instance for each FTE thread.
Invoke firew_log method to send logs via the IPC layer.

On the receiver side, use ipc.NewSharedMem to initialize shared mem.
Use shm.IPCInstance() create a receiver instance and invoke ipc.Receive to
process the messages (see agent/tmagent for a sample).
