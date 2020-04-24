
#ifndef _IONICEVENTLOG_H_
#define _IONICEVENTLOG_H_

#include <ntddk.h>

typedef enum _SEVERITY_EVENTLOG_MSGTYPE {
	EVLOGTYPE_SUCCESS,
	EVLOGTYPE_INFORMATIONAL,
	EVLOGTYPE_WARNING,
	EVLOGTYPE_ERROR
}SEVERITY_EVENTLOG_MSGTYPE, *PSEVERITY_EVENTLOG_MSGTYPE;

#define KERN_EMERG_LOG      EVLOGTYPE_ERROR
#define KERN_ALERT_LOG      EVLOGTYPE_ERROR
#define KERN_CRIT_LOG       EVLOGTYPE_ERROR
#define KERN_ERR_LOG        EVLOGTYPE_ERROR
#define KERN_WARNING_LOG    EVLOGTYPE_WARNING
#define KERN_NOTICE_LOG     EVLOGTYPE_INFORMATIONAL
#define KERN_INFO_LOG       EVLOGTYPE_INFORMATIONAL
#define KERN_DEBUG_LOG      EVLOGTYPE_WARNING
#define KERN_DEFAULT_LOG    EVLOGTYPE_INFORMATIONAL

void FmtMsgWriteToEventLog(PVOID IoObject, ULONG Signature, SEVERITY_EVENTLOG_MSGTYPE EvType, IN PCCHAR DebugMessage, ...);
void WriteToEventLog(PVOID IoObject, ULONG Signature, NTSTATUS Status, PCWSTR wstrExpl);


#define EvLogMsg(lvl,fmt,...)			FmtMsgWriteToEventLog(IonicDriverObject, 'vEoI', lvl, fmt, ##__VA_ARGS__)

#define EvLogError(fmt,...)				EvLogMsg(EVLOGTYPE_ERROR, fmt, ##__VA_ARGS__)
#define EvLogWarning(fmt,...)			EvLogMsg(EVLOGTYPE_WARNING, fmt, ##__VA_ARGS__)
#define EvLogInformational(fmt,...)		EvLogMsg(EVLOGTYPE_INFORMATIONAL, fmt, ##__VA_ARGS__)
#define EvLogSuccess(fmt,...)			EvLogMsg(EVLOGTYPE_SUCCESS, fmt, ##__VA_ARGS__)
#define EvLogStatus(status, fmt,...)	(NT_SUCCESS(status)) ?  EvLogInformational("SUCCESS: " fmt, ##__VA_ARGS__) : EvLogError("0x%x " fmt, status, ##__VA_ARGS__)

#endif