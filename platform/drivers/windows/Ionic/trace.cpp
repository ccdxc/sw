
#include "common.h"

void
DbgLogMsg(IN ULONG Component, IN ULONG Level, IN PCCH Format, ...)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    va_list va_args;
    ULONG ulBytesWritten = 0;
    BOOLEAN bReleaseLock = FALSE;
    char *pCurrentTrace = NULL;

    if (TraceBuffer == NULL) {
        goto cleanup;
    }

    if (Component > 0 && (Component & TraceComponent) == 0) {
        goto cleanup;
    }

    if (Level > 0 && Level > TraceLevel) {
        goto cleanup;
    }

    NdisAcquireSpinLock(&TraceLock);

    bReleaseLock = TRUE;

    //
    // Check again under lock
    //

    if (TraceBuffer == NULL) {
        goto cleanup;
    }

    if (TraceRemainingLength < 255) {
        TraceRemainingLength = TraceBufferLength;
        TraceCurrentBuffer = TraceBuffer;
        SetFlag(TraceFlags, TRACE_BUFFER_WRAPPED);
    }

    pCurrentTrace = TraceCurrentBuffer;

    RtlStringCchPrintfA(TraceCurrentBuffer, 10, "%08lX:", TraceCounter++);

    TraceCurrentBuffer += 9;

    TraceRemainingLength -= 9;

    va_start(va_args, Format);

    ntStatus = RtlStringCbVPrintfA(TraceCurrentBuffer, TraceRemainingLength,
                                   Format, va_args);

    if (ntStatus == STATUS_BUFFER_OVERFLOW) {

        RtlZeroMemory(TraceCurrentBuffer, TraceRemainingLength);

        TraceRemainingLength = TraceBufferLength;

        TraceCurrentBuffer = TraceBuffer;

        SetFlag(TraceFlags, TRACE_BUFFER_WRAPPED);

        pCurrentTrace = TraceCurrentBuffer;

        RtlStringCchPrintfA(TraceCurrentBuffer, 10, "%08lX:", TraceCounter++);

        TraceCurrentBuffer += 9;

        TraceRemainingLength -= 9;

        ntStatus = RtlStringCbVPrintfA(TraceCurrentBuffer, TraceRemainingLength,
                                       Format, va_args);
    }

    if (NT_SUCCESS(ntStatus)) {

        RtlStringCbLengthA(TraceCurrentBuffer, TraceRemainingLength,
                           (size_t *)&ulBytesWritten);

        TraceCurrentBuffer += ulBytesWritten;

        TraceRemainingLength -= ulBytesWritten;
    }

    va_end(va_args);

    if (BooleanFlagOn(TraceFlags, TRACE_TO_DEBUGGER) && pCurrentTrace != NULL) {
        IoPrint(pCurrentTrace);
    }

cleanup:

    if (bReleaseLock) {
        NdisReleaseSpinLock(&TraceLock);
    }

    return;
}

NTSTATUS
InitializeTrace()
{

    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;

    NdisAcquireSpinLock(&TraceLock);

    if (TraceBufferLength > 0) {

        TraceBuffer = (char *)ExAllocatePoolWithTag(
            NonPagedPoolNx, TraceBufferLength, IONIC_GENERIC_TAG);

        if (TraceBuffer != NULL) {
            TraceCurrentBuffer = TraceBuffer;
            TraceRemainingLength = TraceBufferLength;

            InterlockedCompareExchangePointer((void **)&TraceMsgFnc,
                                              (void *)DbgLogMsg, NULL);

            ntStatus = STATUS_SUCCESS;
        }
    }

    NdisReleaseSpinLock(&TraceLock);

    if (NT_SUCCESS(ntStatus)) {
        TagInitialLogEntry();
    }

    return ntStatus;
}

void
TearDownTrace()
{

    NdisAcquireSpinLock(&TraceLock);

    if (TraceBuffer != NULL) {
        ExFreePool(TraceBuffer);
    }

    TraceBuffer = NULL;
    TraceCurrentBuffer = NULL;
    TraceRemainingLength = 0;

    NdisReleaseSpinLock(&TraceLock);

    return;
}

NTSTATUS
ConfigureTrace(IN TraceConfigCB *TraceInfo)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    UNICODE_STRING uniString;
    ULONG ulBufferLen = 0;
    BOOLEAN bUpdateLevel = FALSE;
    BOOLEAN bUpdateComponent = FALSE;
    BOOLEAN bUpdateFlags = FALSE;
    BOOLEAN bUpdateBufferLen = FALSE;

    IoPrint("%s level 0x%lx component 0x%lx\n",
        __FUNCTION__, TraceInfo->Level, TraceInfo->Component);

    NdisAcquireSpinLock(&TraceLock);

    if ((TraceInfo->TraceBufferLength != -1) &&
        (TraceInfo->TraceBufferLength > 10240)) {
        TraceInfo->TraceBufferLength = 10240;
    }

    if (((TraceInfo->Level == -1) || (TraceInfo->Level == TraceLevel)) &&
        ((TraceInfo->TraceBufferLength == -1) ||
         (TraceInfo->TraceBufferLength == (TraceBufferLength / 1024))) &&
        ((TraceInfo->Component == -1) ||
         (TraceInfo->Component == TraceComponent)) &&
        ((TraceInfo->TraceFlags == -1) ||
         (TraceInfo->TraceFlags == TraceFlags))) {

        //
        // Nothing to do
        //
        goto cleanup;
    }

    //
    // Go update the registry with the new entries
    //

    if (TraceInfo->Level != (ULONG)-1 && TraceInfo->Level != TraceLevel) {
        TraceLevel = TraceInfo->Level;
        bUpdateLevel = TRUE;
    }

    if (TraceInfo->Component != (ULONG)-1 &&
        TraceInfo->Component != TraceComponent) {
        TraceComponent = TraceInfo->Component;
        bUpdateComponent = TRUE;
    }

    if (TraceInfo->TraceFlags != (ULONG)-1 &&
        TraceInfo->TraceFlags != TraceFlags) {
        TraceFlags = TraceInfo->TraceFlags;
        bUpdateFlags = TRUE;
    }

    if (TraceInfo->TraceBufferLength != (ULONG)-1 &&
        TraceInfo->TraceBufferLength != (TraceBufferLength / 1024)) {
        ulBufferLen = TraceInfo->TraceBufferLength;
        bUpdateBufferLen = TRUE;

        TraceBufferLength = TraceInfo->TraceBufferLength * 1024;

        ClearFlag(TraceFlags, TRACE_BUFFER_WRAPPED);

        if (TraceBuffer != NULL) {
            ExFreePool(TraceBuffer);
            TraceBuffer = NULL;

            TraceCurrentBuffer = NULL;
            TraceRemainingLength = 0;
        }

        if (TraceBufferLength > 0) {

            TraceBuffer = (char *)ExAllocatePoolWithTag(
                NonPagedPoolNx, TraceBufferLength, IONIC_GENERIC_TAG);

            if (TraceBuffer == NULL) {
                TraceBufferLength = 0;
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }

            TraceCurrentBuffer = TraceBuffer;

            TraceRemainingLength = TraceBufferLength;

            InterlockedCompareExchangePointer((void **)&TraceMsgFnc,
                                              (void *)DbgLogMsg, NULL);
        } else {
            InterlockedCompareExchangePointer((void **)&TraceMsgFnc, NULL,
                                              (void *)DbgLogMsg);
        }
    }

cleanup:

    NdisReleaseSpinLock(&TraceLock);

    if (bUpdateLevel) {

        RtlInitUnicodeString(&uniString, REG_TRACE_LEVEL);

        ntStatus = UpdateRegistryParameter(&uniString, REG_DWORD, &TraceLevel,
                                           sizeof(ULONG));

        if (!NT_SUCCESS(ntStatus)) {
            IoPrint("ConfigureTrace Failed to set debug level in registry "
                    "Status %08lX\n",
                    ntStatus);
        }
    }

    if (bUpdateComponent) {

        RtlInitUnicodeString(&uniString, REG_TRACE_COMPONENT);

        ntStatus = UpdateRegistryParameter(&uniString, REG_DWORD,
                                           &TraceComponent, sizeof(ULONG));

        if (!NT_SUCCESS(ntStatus)) {
            IoPrint("ConfigureTrace Failed to set debug subsystem in registry "
                    "Status %08lX\n",
                    ntStatus);
        }
    }

    if (bUpdateFlags) {

        RtlInitUnicodeString(&uniString, REG_TRACE_FLAGS);

        ntStatus = UpdateRegistryParameter(&uniString, REG_DWORD, &TraceFlags,
                                           sizeof(ULONG));

        if (!NT_SUCCESS(ntStatus)) {
            IoPrint("ConfigureTrace Failed to set debug flags in registry "
                    "Status %08lX\n",
                    ntStatus);
        }
    }

    if (bUpdateBufferLen) {

        RtlInitUnicodeString(&uniString, REG_TRACE_BUFFER_LENGTH);

        ntStatus = UpdateRegistryParameter(&uniString, REG_DWORD, &ulBufferLen,
                                           sizeof(ULONG));

        if (!NT_SUCCESS(ntStatus)) {
            IoPrint("ConfigureTrace Failed to set debug buffer length in "
                    "registry Status %08lX\n",
                    ntStatus);
        }

        if (TraceBufferLength > 0) {
            TagInitialLogEntry();
        }
    }

    return ntStatus;
}

NTSTATUS
GetTraceBuffer(IN ULONG BufferLength, OUT void *Buffer, OUT ULONG *CopiedLength)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG ulCopyLength = 0;
    char *pCurrentLocation = NULL;
    void *pTmpBuffer = NULL;

    *CopiedLength = 0;

    NdisAcquireSpinLock(&TraceLock);

    if (TraceBufferLength == 0) {
        *CopiedLength = 0;
        goto cleanup;
    }

    if (BufferLength < TraceBufferLength) {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    pTmpBuffer = ExAllocatePoolWithTag(NonPagedPoolNx, TraceBufferLength,
                                       IONIC_GENERIC_TAG);

    if (pTmpBuffer == NULL) {
        goto cleanup;
    }

    //
    // If we have wrapped then copy in the remaining portion
    //

    pCurrentLocation = (char *)pTmpBuffer;

    if (BooleanFlagOn(TraceFlags, TRACE_BUFFER_WRAPPED)) {

        ulCopyLength = TraceRemainingLength;

        RtlCopyMemory(pCurrentLocation, TraceCurrentBuffer, ulCopyLength);

        pCurrentLocation[0] = '0'; // The buffer is NULL terminated ...

        pCurrentLocation += ulCopyLength;

        *CopiedLength = ulCopyLength;
    }

    ulCopyLength = TraceBufferLength - TraceRemainingLength;

    if (ulCopyLength > 0) {

        RtlCopyMemory(pCurrentLocation, TraceBuffer, ulCopyLength);

        *CopiedLength += ulCopyLength;
    }

cleanup:

    NdisReleaseSpinLock(&TraceLock);

    if (*CopiedLength != 0) {
        RtlCopyMemory(Buffer, pTmpBuffer, *CopiedLength);
    }

    if (pTmpBuffer != NULL) {
        ExFreePool(pTmpBuffer);
    }

    return ntStatus;
}

void
TagInitialLogEntry()
{

    LARGE_INTEGER liTime, liLocalTime;
    TIME_FIELDS timeFields;

    KeQuerySystemTime(&liTime);

    ExSystemTimeToLocalTime(&liTime, &liLocalTime);

    RtlTimeToTimeFields(&liLocalTime, &timeFields);

    DbgTrace((0, 0,
              "Ionic System Log Initialized %d-%d-%d %d:%d Level %d Subsystems "
              "%08lX\n",
              timeFields.Month, timeFields.Day, timeFields.Year,
              timeFields.Hour, timeFields.Minute, TraceLevel, TraceComponent));
    return;
}
