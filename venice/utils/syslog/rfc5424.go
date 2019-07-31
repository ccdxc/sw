package syslog

import (
	"context"
	"fmt"
	"net"
	"os"
	"strings"
	"sync"
	"time"

	syslog "github.com/RackSec/srslog"

	"github.com/pensando/sw/venice/utils"
)

// TODO: handle connection failure

var logTypes = []string{"unixgram", "unix"}
var logPaths = []string{"/dev/log", "/var/run/syslog", "/var/run/log"}

// ROption to be added to the config
type ROption func(w *rfc5424)

// RFC5424 format  syslog writer
type rfc5424 struct {
	sync.Mutex
	version  int
	facility syslog.Priority
	hostname string
	appName  string
	procID   int
	conn     net.Conn
	getTime  func() string
	network  string
	raddr    string
	ctx      context.Context
}

// WithTimeFn adds time fn to the config
func WithTimeFn(fn func() string) ROption {
	return func(w *rfc5424) {
		w.getTime = fn
	}
}

// WithConn adds conn to the the config
func WithConn(conn net.Conn) ROption {
	return func(w *rfc5424) {
		w.conn = conn
	}
}

// RFCWithContext passes a context for RFC writer
func RFCWithContext(ctx context.Context) ROption {
	return func(w *rfc5424) {
		w.ctx = ctx
	}
}

// NewRfc5424 creates instance to write syslog in RFC5424 format
func NewRfc5424(network, raddr string, facility Priority, hostname, appName string, opts ...ROption) (Writer, error) {
	if appName == "" {
		appName = "-"
	}

	if hostname == "" {
		hostname = utils.GetHostname()
	}

	w := &rfc5424{
		version:  1,
		facility: syslog.Priority(facility),
		appName:  appName,
		hostname: hostname,
		procID:   os.Getpid(),
		getTime:  func() string { return time.Now().Format(time.RFC3339) },
		network:  network,
		raddr:    raddr,
	}

	w.Lock()
	defer w.Unlock()

	// add custom options
	for _, o := range opts {
		if o != nil {
			o(w)
		}
	}

	if w.conn == nil {
		conn, err := w.connect()
		if err != nil {
			return nil, err
		}
		w.conn = conn
	}

	return w, nil
}

func (w *rfc5424) connect() (net.Conn, error) {
	var conn net.Conn
	var err error
	errMsg := []string{}

	if !utils.IsEmpty(w.network) || !utils.IsEmpty(w.raddr) {
		dialer := &net.Dialer{Timeout: Timeout}
		if w.ctx != nil {
			conn, err = dialer.DialContext(w.ctx, w.network, w.raddr)
		} else {
			conn, err = dialer.Dial(w.network, w.raddr)
		}
		if err != nil {
			return nil, err
		}

		return conn, nil
	}

	// if there is no remote server, connect to the local syslog
	for _, network := range logTypes {
		for _, path := range logPaths {
			dialer := &net.Dialer{Timeout: Timeout}
			if w.ctx != nil {
				conn, err = dialer.DialContext(w.ctx, network, path)
			} else {
				conn, err = dialer.Dial(network, path)
			}
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

// Close closes a connection to the syslog daemon
func (w *rfc5424) Close() error {
	w.Lock()
	defer w.Unlock()
	if w.conn != nil {
		w.conn.Close()
	}
	return nil
}

// Emerg logs a message with severity LOG_EMERG
func (w *rfc5424) Emerg(msg *Message) error {
	return w.writeAndRetry(syslog.LOG_EMERG, msg.MsgID, msg.StructuredData, msg.Msg)
}

// Alert logs a message with severity LOG_ALERT
func (w *rfc5424) Alert(msg *Message) error {
	return w.writeAndRetry(syslog.LOG_ALERT, msg.MsgID, msg.StructuredData, msg.Msg)
}

// Crit logs a message with severity LOG_CRIT
func (w *rfc5424) Crit(msg *Message) error {
	return w.writeAndRetry(syslog.LOG_CRIT, msg.MsgID, msg.StructuredData, msg.Msg)
}

// Err logs a message with severity LOG_ERR
func (w *rfc5424) Err(msg *Message) error {
	return w.writeAndRetry(syslog.LOG_ERR, msg.MsgID, msg.StructuredData, msg.Msg)
}

// Warning logs a message with severity LOG_WARNING
func (w *rfc5424) Warning(msg *Message) error {
	return w.writeAndRetry(syslog.LOG_WARNING, msg.MsgID, msg.StructuredData, msg.Msg)
}

// Notice logs a message with severity LOG_NOTICE
func (w *rfc5424) Notice(msg *Message) error {
	return w.writeAndRetry(syslog.LOG_NOTICE, msg.MsgID, msg.StructuredData, msg.Msg)
}

// Info logs a message with severity LOG_INFO
func (w *rfc5424) Info(msg *Message) error {
	return w.writeAndRetry(syslog.LOG_INFO, msg.MsgID, msg.StructuredData, msg.Msg)
}

// Debug logs a message with severity LOG_DEBUG
func (w *rfc5424) Debug(msg *Message) error {
	return w.writeAndRetry(syslog.LOG_DEBUG, msg.MsgID, msg.StructuredData, msg.Msg)
}

func (w *rfc5424) writeAndRetry(severity syslog.Priority, msgID string, data StrData, msg string) error {
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

	logMsg += data.stringify()
	logMsg += fmt.Sprintf(" %s", msg)

	w.Lock()
	defer w.Unlock()

	var err error
	if w.conn != nil { // write
		if _, err = fmt.Fprint(w.conn, logMsg); err == nil {
			return nil
		}
	}

	if w.conn, err = w.connect(); err != nil { // retry
		return err
	}
	_, err = fmt.Fprint(w.conn, logMsg)
	return err
}
