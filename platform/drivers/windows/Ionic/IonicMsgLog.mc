;/*--
;Module Name:
;    IonicMsgLog.mc
;Abstract:
;    Constant definitions for the I/O error code log values.
;--*/
;
;#ifndef _IONICMSGLOG_
;#define _IONICMSGLOG_
;
;//  Status values are 32 bit values layed out as follows:
;//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
;//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
;//  +---+-+-------------------------+-------------------------------+
;//  |Sev|C|       Facility          |               Code            |
;//  +---+-+-------------------------+-------------------------------+
;//  where
;//      Sev - is the severity code
;//          00 - Success
;//          01 - Informational
;//          10 - Warning
;//          11 - Error
;//      C - is the Customer code flag
;//      Facility - is the facility code
;//      Code - is the facility's status code
;
MessageIdTypedef=NTSTATUS
SeverityNames = (
	Success			= 0x0:STATUS_SEVERITY_SUCCESS
	Informational	= 0x1:STATUS_SEVERITY_INFORMATIONAL
	Warning			= 0x2:STATUS_SEVERITY_WARNING
	Error			= 0x3:STATUS_SEVERITY_ERROR
	)

FacilityNames = (
	System			= 0x0
	Ionicdriver 	= 0x55:FACILITY_IONICEVLOG_ERROR_CODE
	)

LanguageNames = (
	English			= 0x0409:msg00409
	)

MessageId = 0x0001
Facility = Ionicdriver
Severity = Success
SymbolicName = IONICEVLOG_GENERIC_SUCCESS
Language = English
SUCCESS: %2
.

MessageId = 0x0002
Facility = Ionicdriver
Severity = Informational
SymbolicName = IONICEVLOG_GENERIC_INFORMATIONAL
Language = English
INFORMATIONAL: %2
.

MessageId = 0x0003
Facility = Ionicdriver
Severity = Warning
SymbolicName = IONICEVLOG_GENERIC_WARNING
Language = English
WARNING: %2
.

MessageId = 0x0004
Facility = Ionicdriver
Severity = Error
SymbolicName = IONICEVLOG_GENERIC_ERROR
Language = English
ERROR: %2
.



;#endif  // _IONICMSGLOG_
