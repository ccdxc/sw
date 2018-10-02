package syslog

import (
	"regexp"

	"github.com/pensando/sw/api/generated/monitoring"
)

var (
	pri       = `<([0-9]{1,3})>`
	ts        = `([^ ]+)`
	host      = `([^ ]+)`
	tag       = `([\w-]+)`
	pid       = `([0-9]{1,5})`
	msg       = `(.+$)`
	bsdFormat = regexp.MustCompile(`^` + pri + ts + `\s` + host + `\s` + tag + `\[` + pid + `\]` + `:\s` + msg)

	ver           = `([0-9])`
	app           = `([^ ]+)`
	msgID         = `([\w-]+)`
	rfc5424Format = regexp.MustCompile(`^` + pri + ver + `\s` + ts + `\s` + host + `\s` + app + `\s` + pid + `\s` + msgID + `\s` + msg)
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
// e.g. <134>2018-09-21T17:45:16-07:00 Yuvas-MBP.pensando.io tag1[74544]: test2
func validateBSDMessage(msg string) bool {
	return len(bsdFormat.FindStringSubmatch(msg)) == 7
}

// validates RFC5424 formatted message
// e.g. <94>1 2018-09-24T14:16:21-07:00 host1 app1 78990  [id1 k1="v1"] test1
func validateRFC5424Message(msg string) bool {
	return len(rfc5424Format.FindStringSubmatch(msg)) == 9
}
