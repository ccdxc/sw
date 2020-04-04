
#include "common.h"
#include "IonicMsgLog.h"

//#include <ntddk.h>
//#include <ntstrsafe.h>

static __inline NTSTATUS MyAnsiStringToUnicodeString(_Out_ PUNICODE_STRING DestinationString, _In_ PCANSI_STRING SourceString) {
	USHORT i;
	if ((NULL == SourceString) || (NULL == DestinationString)) {
		return STATUS_INVALID_PARAMETER;
	}
	if ((NULL == DestinationString->Buffer) ||
		(NULL == SourceString->Buffer) ||
		(DestinationString->MaximumLength < sizeof(WCHAR)) ||
		(SourceString->MaximumLength < sizeof(CHAR)) ||
		(SourceString->Length >= SourceString->MaximumLength)) {
		return STATUS_INVALID_PARAMETER;
	}
	DestinationString->Length = 0;
	RtlZeroMemory(DestinationString->Buffer, DestinationString->MaximumLength);
	for (i = 0; i < SourceString->Length; i++) {
		if ((i + 1) * sizeof(WCHAR) >= DestinationString->MaximumLength) {
			break;
		}
		if (0 == SourceString->Buffer[i]) {
			break;
		}
		DestinationString->Buffer[i] = (WCHAR)SourceString->Buffer[i];
		DestinationString->Length += sizeof(WCHAR);
	}
	return STATUS_SUCCESS;
}

static __inline NTSTATUS ConvertCharToWideChar(IN PCSTR pstrChar, OUT PWSTR pstrWchar, IN USHORT strChMaxSize) {
	UNICODE_STRING ustr;
	ANSI_STRING astr;

	RtlInitAnsiString(&astr, pstrChar);

	pstrWchar[0] = 0;
	ustr.Buffer = pstrWchar;
	ustr.Length = 0;
	ustr.MaximumLength = strChMaxSize * sizeof(WCHAR);

	// RtlAnsiStringToUnicodeString can only execute at PASSIVE_LEVEL
	// Since we possibly calling ConvertCharToWideChar at DISPATCH_LEVEL, we cannot call RtlAnsiStringToUnicodeString
	//return RtlAnsiStringToUnicodeString(&ustr, &astr, FALSE);
	return MyAnsiStringToUnicodeString(&ustr, &astr);
}


#define MAX_LOG_BUFFER_CHAR	90

void FmtMsgWriteToEventLog(PVOID IoObject, ULONG Signature, SEVERITY_EVENTLOG_MSGTYPE EvType, IN PCCHAR DebugMessage, ...) {
    va_list    list;
    CHAR       AnsiString[MAX_LOG_BUFFER_CHAR];
    WCHAR      EvLogMessageBuffer[MAX_LOG_BUFFER_CHAR];
    NTSTATUS   status;

    va_start(list, DebugMessage);

    if (DebugMessage) {
        status = RtlStringCbVPrintfA( AnsiString, sizeof(AnsiString), DebugMessage, list );
		if (STATUS_BUFFER_OVERFLOW == status) {
			AnsiString[MAX_LOG_BUFFER_CHAR-1] = 0;
		} else if (!NT_SUCCESS(status)) {
            KdPrint((": RtlStringCbVPrintfA failed 0x%x\n", status));
            return;
        }

		status = ConvertCharToWideChar(AnsiString, EvLogMessageBuffer, MAX_LOG_BUFFER_CHAR);
		if (STATUS_SUCCESS != status) {
            KdPrint((": ConvertCharToWideChar failed 0x%x\n", status));
            return;
		}
		switch(EvType) {
		case EVLOGTYPE_SUCCESS: status = IONICEVLOG_GENERIC_SUCCESS; break;
		case EVLOGTYPE_INFORMATIONAL: status = IONICEVLOG_GENERIC_INFORMATIONAL; break;
		case EVLOGTYPE_WARNING: status = IONICEVLOG_GENERIC_WARNING; break;
		case EVLOGTYPE_ERROR:
		default:
			status = IONICEVLOG_GENERIC_ERROR; break;
		}

		KdPrint(("Log message: %s\n", AnsiString));

		WriteToEventLog(IoObject, Signature, status, EvLogMessageBuffer);
    }
    va_end(list);
}

void WriteToEventLog(PVOID IoObject, ULONG Signature, NTSTATUS Status, PCWSTR wstrExpl){
ULONG PacketLen, DumpDataLen;
PIO_ERROR_LOG_PACKET p;
	DumpDataLen = sizeof(ULONG);
	if (wstrExpl)
		DumpDataLen += (ULONG)(wcslen(wstrExpl) + 1) * sizeof(WCHAR);
	PacketLen = sizeof(IO_ERROR_LOG_PACKET) + DumpDataLen;

	if (PacketLen > ERROR_LOG_MAXIMUM_SIZE){
		KdPrint(("Size to big: %d, max: %d\n", PacketLen, ERROR_LOG_MAXIMUM_SIZE));
		return;
	}
	p = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(IoObject, (UCHAR)PacketLen);
	if (!p){
		KdPrint(("IoAllocateErrorLogEntry failed\n"));
		return;
	}
	RtlZeroMemory(p, PacketLen);
	p->ErrorCode =  Status;
	p->DumpDataSize = sizeof(ULONG);
	p->DumpData[0] = Signature;
	p->StringOffset = sizeof(IO_ERROR_LOG_PACKET) - sizeof(ULONG) + p->DumpDataSize;
	if (wstrExpl){
		p->NumberOfStrings = 1;
		RtlStringCbCopyW((PWSTR) ((PUCHAR) p + p->StringOffset), (wcslen(wstrExpl) + 1) * sizeof(WCHAR), wstrExpl);
	}
	IoWriteErrorLogEntry(p);
}


