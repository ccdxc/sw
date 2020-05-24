
#ifndef _IONICWORKERTHREAD_H_
#define _IONICWORKERTHREAD_H_

#include <ntddk.h>

// Worker thread
// -------------------------------------------------------------------------
typedef struct _IONIC_WORKER_THREAD {
    KEVENT      evStopThread;       // notifies the worker thread to exit
    KEVENT      evThreadDoWork;     // notifies the worker thread to do some work

    PKTHREAD    threadWorker;       // worker thread
    BOOLEAN     bThreadStarted;     // set to TRUE if the worker thread was started successfuly
    BOOLEAN     bInitialized;       // set to TRUE when thread is initialized
} IONIC_WORKER_THREAD, *PIONIC_WORKER_THREAD;

VOID InitWorkerThread(PIONIC_WORKER_THREAD pIonicWorkerThread);

VOID StopWorkerThread(PIONIC_WORKER_THREAD pIonicWorkerThread);

NTSTATUS StartWorkerThread(PIONIC_WORKER_THREAD pIonicWorkerThread, PKSTART_ROUTINE pThreadProc, PVOID Context);

#endif
