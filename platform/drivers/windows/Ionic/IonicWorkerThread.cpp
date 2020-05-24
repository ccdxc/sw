#include "common.h"

VOID InitWorkerThread(PIONIC_WORKER_THREAD pIonicWorkerThread) {
    // allow reinitialize
    if (pIonicWorkerThread->bInitialized) {
        // stop the thread if is still running
        StopWorkerThread(pIonicWorkerThread);
    }
    KeInitializeEvent(&pIonicWorkerThread->evStopThread, NotificationEvent, FALSE);
    KeInitializeEvent(&pIonicWorkerThread->evThreadDoWork, SynchronizationEvent, FALSE);
    pIonicWorkerThread->threadWorker = NULL;
    pIonicWorkerThread->bThreadStarted = FALSE;

    pIonicWorkerThread->bInitialized = TRUE;

}

NTSTATUS StartWorkerThread(PIONIC_WORKER_THREAD pIonicWorkerThread, PKSTART_ROUTINE pThreadProc, PVOID Context) {
    NTSTATUS status;
    HANDLE hthread;
    OBJECT_ATTRIBUTES oa;

    DbgTrace((TRACE_COMPONENT_WORKER_THREAD, TRACE_LEVEL_VERBOSE, "%s Irql Level: %u\n", __FUNCTION__, KeGetCurrentIrql()));

    if (NULL != pThreadProc) {
        InitializeObjectAttributes(&oa, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
        // creating app notification thread
        status = PsCreateSystemThread(&hthread, THREAD_ALL_ACCESS, &oa, NULL, NULL, pThreadProc, Context);
        if (STATUS_SUCCESS != status) {
            DbgTrace((TRACE_COMPONENT_WORKER_THREAD, TRACE_LEVEL_ERROR, "%s Failed to create worker thread. Status: %x\n", __FUNCTION__, status));
            return status;
        }
        status = ObReferenceObjectByHandle(hthread, THREAD_ALL_ACCESS, NULL, KernelMode, (PVOID*)&pIonicWorkerThread->threadWorker, NULL);
        if (STATUS_SUCCESS != status) {
            DbgTrace((TRACE_COMPONENT_WORKER_THREAD, TRACE_LEVEL_ERROR, "%s Failed to reference object by handle. Status: %x\n", __FUNCTION__, status));
            KeSetEvent(&pIonicWorkerThread->evStopThread, 0, FALSE);
            return status;
        }
        ZwClose(hthread);
        pIonicWorkerThread->bThreadStarted = TRUE;
        DbgTrace((TRACE_COMPONENT_WORKER_THREAD, TRACE_LEVEL_VERBOSE, "%s return status: %x\n", __FUNCTION__, status));
        return status;
    }
    else {
        // ...
        return STATUS_SUCCESS;
    }
}


VOID StopWorkerThread(PIONIC_WORKER_THREAD pIonicWorkerThread) {
    
    DbgTrace((TRACE_COMPONENT_WORKER_THREAD, TRACE_LEVEL_VERBOSE, "%s Irql Level: %u\n", __FUNCTION__, KeGetCurrentIrql()));
    
    KeSetEvent(&pIonicWorkerThread->evStopThread, 0, FALSE);
    if (pIonicWorkerThread->bThreadStarted) {
        
        NTSTATUS WaitStatus = KeWaitForSingleObject(pIonicWorkerThread->threadWorker, Executive, KernelMode, FALSE, NULL);
        UNREFERENCED_PARAMETER(WaitStatus);

        ASSERT(WaitStatus == STATUS_SUCCESS);
        
        ObDereferenceObject(pIonicWorkerThread->threadWorker);
        
        pIonicWorkerThread->bThreadStarted = FALSE;
    }
    KeClearEvent(&pIonicWorkerThread->evStopThread);
}
