package syslog

import (
	"fmt"
	"time"

	syslog "github.com/RackSec/srslog"
)

// StrData represents structured data within a log message
type StrData map[string]map[string]string

// Priority is the syslog priority
type Priority syslog.Priority

// Severity.
const (
	LogEmerg Priority = iota
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
	LogKern Priority = iota << 3
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

// net.Dial timeout
const (
	Timeout = 4 * time.Second
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

// ValidateFacility vaidates syslog facility
func ValidateFacility(p Priority) error {
	if p < 0 || p > LogLocal7 {
		return fmt.Errorf("log/syslog: invalid facility")
	}
	return nil
}

// WriterConfig is a wrapper that includes syslog config for writer
type WriterConfig struct {
	Format     string
	Network    string
	RemoteAddr string
	Priority   Priority
	Tag        string
	Writer     Writer
}
