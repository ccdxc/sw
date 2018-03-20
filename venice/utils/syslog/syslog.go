package syslog

import (
	"fmt"
	"log/syslog"
	"net"
	"os"
	"strings"
	"sync"
	"time"
)

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

var logTypes = []string{"unixgram", "unix"}
var logPaths = []string{"/dev/log", "/var/run/syslog", "/var/run/log"}

// Writer RFC5424 syslog writer
type Writer struct {
	sync.Mutex
	version   int
	facility  syslog.Priority
	hostname  string
	appName   string
	procID    int
	conn      net.Conn
	messageID string
	getTime   func() string
}

// StrData represents structured data within a log message
type StrData map[string]map[string]string

// NewBsd is a wrapper around bsd syslog
func NewBsd(priority syslog.Priority, tag string) (*syslog.Writer, error) {
	return syslog.New(priority, tag)
}

// NewRfc5424 creates instance to write syslog in RFC5424 format
func NewRfc5424(facility syslog.Priority, hostName, appName string) (*Writer, error) {
	if appName == "" {
		appName = "-"
	}

	if hostName == "" {
		hostName = func() string {
			if h, err := os.Hostname(); err == nil {
				return h
			}
			return "-"
		}()
	}

	w := &Writer{
		version:  1,
		facility: facility,
		appName:  appName,
		hostname: hostName,
		procID:   os.Getpid(),
		getTime:  func() string { return time.Now().Format(time.RFC3339) },
	}

	w.Lock()
	defer w.Unlock()

	conn, err := w.connect()
	if err != nil {
		return nil, err
	}

	w.conn = conn
	return w, nil
}

func (w *Writer) connect() (net.Conn, error) {
	errMsg := []string{}

	for _, network := range logTypes {
		for _, path := range logPaths {
			conn, err := net.Dial(network, path)
			if err != nil {
				errMsg = append(errMsg, fmt.Sprintf("%s:%s %s\n", network, path, err))
				continue
			} else {
				return conn, nil
			}
		}
	}
	return nil, fmt.Errorf("%v", errMsg)
}

// Close closes a connection to the syslog daemon.
func (w *Writer) Close() {
	w.Lock()
	defer w.Unlock()
	w.conn.Close()
}

// Emerg logs a message with severity LOG_EMERG
func (w *Writer) Emerg(msgID string, data StrData, m string) error {
	return w.write(syslog.LOG_EMERG, msgID, data, m)
}

// Alert logs a message with severity LOG_ALERT
func (w *Writer) Alert(msgID string, data StrData, m string) error {
	return w.write(syslog.LOG_ALERT, msgID, data, m)
}

// Crit logs a message with severity LOG_CRIT
func (w *Writer) Crit(msgID string, data StrData, m string) error {
	return w.write(syslog.LOG_CRIT, msgID, data, m)
}

// Err logs a message with severity LOG_ERR
func (w *Writer) Err(msgID string, data StrData, m string) error {
	return w.write(syslog.LOG_ERR, msgID, data, m)
}

// Warning logs a message with severity LOG_WARNING
func (w *Writer) Warning(msgID string, data StrData, m string) error {
	return w.write(syslog.LOG_WARNING, msgID, data, m)
}

// Notice logs a message with severity LOG_NOTICE
func (w *Writer) Notice(msgID string, data StrData, m string) error {
	return w.write(syslog.LOG_NOTICE, msgID, data, m)
}

// Info logs a message with severity LOG_INFO
func (w *Writer) Info(msgID string, data StrData, m string) error {
	return w.write(syslog.LOG_INFO, msgID, data, m)
}

// Debug logs a message with severity LOG_DEBUG
func (w *Writer) Debug(msgID string, data StrData, m string) error {
	return w.write(syslog.LOG_DEBUG, msgID, data, m)
}

func (w *Writer) write(severity syslog.Priority, msgID string, data StrData, msg string) error {
	pr := (w.facility & 0xF8) | (severity & 0x07)
	strCheck := func(s string) string {
		if s == "" {
			return "-"
		}
		return s
	}

	// ensure it ends in a \n
	if !strings.HasSuffix(msg, "\n") {
		msg += "\n"
	}

	logMsg := fmt.Sprintf("<%d>%d %s %s %s %d %s ",
		pr, w.version,
		w.getTime(),
		strCheck(w.hostname),
		strCheck(w.appName),
		w.procID,
		msgID)

	if data != nil {
		for sdKey, sdParam := range data {
			logMsg += fmt.Sprintf("[%s", sdKey)
			for k, v := range sdParam {
				logMsg += fmt.Sprintf(" %s=\"%s\"", k, v)
			}
			logMsg += fmt.Sprintf("]")
		}
	} else {
		logMsg += "-"
	}

	logMsg += fmt.Sprintf(" %s", msg)

	w.Lock()
	defer w.Unlock()

	if w.conn == nil {
		conn, err := w.connect()
		if err != nil {
			return err
		}
		w.conn = conn
	}
	_, err := fmt.Fprint(w.conn, logMsg)

	return err
}
