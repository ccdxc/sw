package syslog

import "log/syslog"

// StrData represents structured data within a log message
type StrData map[string]map[string]string

// Severity.
const (
	LogEmerg syslog.Priority = iota
	LogAlert
	LogCrit
	LogErr
	LogWarning
	LogNotice
	LogInfo
	LogDebug
)

// Facility.
const (
	LogKern syslog.Priority = iota << 3
	LogUser
	LogMail
	LogDaemon
	LogAuth
	LogSyslog
	LogLpr
	LogNews
	LogUucp
	LogCron
	LogAuthPriv
	LogFtp
	_ // unused
	_ // unused
	_ // unused
	_ // unused
	LogLocal0
	LogLocal1
	LogLocal2
	LogLocal3
	LogLocal4
	LogLocal5
	LogLocal6
	LogLocal7
)

// Message represents the syslog message
type Message struct {
	MsgID          string
	StructuredData StrData
	Msg            string
}

// Writer represents the syslog writer
type Writer interface {
	Emerg(*Message) error
	Alert(*Message) error
	Crit(*Message) error
	Err(*Message) error
	Warning(*Message) error
	Notice(*Message) error
	Info(*Message) error
	Debug(*Message) error
	Close() error
}
