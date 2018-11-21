package syslog

import (
	"regexp"

	"github.com/pensando/sw/api/generated/monitoring"
)

var (
	pri       = `<([0-9]{1,3})>`
	ts3164    = `([A-Z][a-z][a-z]\s{1,2}\d{1,2}\s\d{2}[:]\d{2}[:]\d{2})`
	host      = `([^ ]+)`
	tag       = `([\w-]+)`
	pid       = `([0-9]{1,5})`
	lmsg      = `(.+$)`
	bsdFormat = regexp.MustCompile(`^` + pri + ts3164 + `\s` + host + `\s` + tag + `\[` + pid + `\]` + `:\s` + lmsg)

	ts5424        = `(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:[+-]\d{2}:\d{2}|\d*Z))`
	ver           = `([0-9])`
	app           = `([^ ]+)`
	msgID         = `([\w-]+)`
	rfc5424Format = regexp.MustCompile(`^` + pri + ver + `\s` + ts5424 + `\s` + host + `\s` + app + `\s` + pid + `\s` + msgID + `\s` + lmsg)
)

// ValidateSyslogMessage validates the given syslog message format
func ValidateSyslogMessage(format monitoring.MonitoringExportFormat, msg string) bool {
	switch format {
	case monitoring.MonitoringExportFormat_SYSLOG_BSD:
		return validateBSDMessage(msg)
	case monitoring.MonitoringExportFormat_SYSLOG_RFC5424:
		return validateRFC5424Message(msg)
	}

	return false
}

// validates BSD formatted message
// example: <131>Nov 19 10:55:07 Pensando-MBP.pensando.io tag1[96795]: test2
func validateBSDMessage(msg string) bool {
	return len(bsdFormat.FindStringSubmatch(msg)) == 7
}

// validates RFC5424 formatted message
// e.g. <94>1 2018-09-24T14:16:21-07:00 host1 app1 78990  [id1 k1="v1"] test1
// e.g. <94>1 2018-11-19T21:05:36Z
func validateRFC5424Message(msg string) bool {
	return len(rfc5424Format.FindStringSubmatch(msg)) == 9
}
