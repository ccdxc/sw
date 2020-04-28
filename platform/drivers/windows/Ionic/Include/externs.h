#pragma once

extern IONIC_VERSION_INFO IonicVersionInfo;

extern PDRIVER_OBJECT   IonicDriverObject;

extern NDIS_HANDLE		IonicDriver;

extern unsigned int		devcmd_timeout;

extern unsigned int		max_slaves;

extern unsigned int		rx_copybreak;

extern NDIS_HANDLE		AdapterControl;

extern DEVICE_OBJECT   *AdapterCntrlDevObj;

extern UNICODE_STRING	AdapterRegistryPath;

extern ULONG            TraceFlags;

extern ULONG            TraceComponent;

extern ULONG            TraceLevel;

extern NDIS_SPIN_LOCK   TraceLock;

extern ULONG            TraceRemainingLength;

extern char            *TraceCurrentBuffer;

extern char            *TraceBuffer;

extern ULONG            TraceCounter;

extern ULONG            TraceBufferLength;

extern PTraceLogMsg     TraceMsgFnc;

extern LIST_ENTRY		AdapterList;

extern NDIS_MUTEX	AdapterListLock;

extern LONG				port_count;

extern ULONG			RxBudget;

extern ULONG            StateFlags;

extern KEVENT           perfmon_event;

extern ULONG            PerfMonInitialized;

extern LIST_ENTRY       memory_block_list;

extern NDIS_SPIN_LOCK   memory_block_lock;