package syslog

import (
	"context"
	"encoding/json"
	"fmt"
	"net"

	syslog "github.com/RackSec/srslog"

	"github.com/pensando/sw/venice/utils"
)

// TODO: handle connection failure

// BSD format syslog writer
type bsd struct {
	sw       *syslog.Writer
	network  string // "tcp", "tcp4" (IPv4-only), "tcp6" (IPv6-only), "udp", "udp4" (IPv4-only), "udp6" (IPv6-only), "ip", "ip4" (IPv4-only), "ip6" (IPv6-only), "unix", "unixgram" and "unixpacket".
	raddr    string // remote addr to connect to
	hostname string // hostname to record the syslog message with
	ctx      context.Context
}

// BOption fills the optional params for bsd syslog writer
type BOption func(*bsd)

// BSDWithContext passes a context for bsd writer
func BSDWithContext(ctx context.Context) BOption {
	return func(b *bsd) {
		b.ctx = ctx
	}
}

// NewBsd is a wrapper around bsd syslog
func NewBsd(network, raddr string, priority Priority, hostname, tag string, opts ...BOption) (Writer, error) {
	if hostname == "" {
		hostname = utils.GetHostname()
	}

	b := &bsd{network: network, raddr: raddr, hostname: hostname}

	// add custom options
	for _, o := range opts {
		if o != nil {
			o(b)
		}
	}

	dialWithContext := func(network, addr string) (net.Conn, error) {
		dialer := &net.Dialer{Timeout: Timeout}
		if b.ctx != nil {
			return dialer.DialContext(b.ctx, network, addr)
		}
		return dialer.Dial(network, addr)
	}

	sw, err := syslog.DialWithCustomDialer(network, raddr, syslog.Priority(priority), tag, dialWithContext)
	if err != nil {
		return nil, err
	}
	sw.SetFormatter(syslog.RFC3164Formatter)
	sw.SetHostname(hostname)
	b.sw = sw

	return b, nil
}

// Close closes the underlying client connection to the syslog server
func (w *bsd) Close() error {
	if w.sw != nil {
		w.sw.Close()
	}
	return nil
}

// Emerg logs a message with severity LOG_EMERG
func (w *bsd) Emerg(msg *Message) error {
	message := fmt.Sprintf("%s", msg.Msg)
	if msg.StructuredData != nil {
		m, err := json.Marshal(msg.StructuredData)
		if err != nil {
			return err
		}
		message = fmt.Sprintf("%s - %s", msg.Msg, string(m))
	}
	return w.sw.Emerg(message)
}

// Alert logs a message with severity LOG_ALERT
func (w *bsd) Alert(msg *Message) error {
	message := fmt.Sprintf("%s", msg.Msg)
	if msg.StructuredData != nil {
		m, err := json.Marshal(msg.StructuredData)
		if err != nil {
			return err
		}
		message = fmt.Sprintf("%s - %s", msg.Msg, string(m))
	}
	return w.sw.Alert(message)
}

// Crit logs a message with severity LOG_CRIT
func (w *bsd) Crit(msg *Message) error {
	message := fmt.Sprintf("%s", msg.Msg)
	if msg.StructuredData != nil {
		m, err := json.Marshal(msg.StructuredData)
		if err != nil {
			return err
		}
		message = fmt.Sprintf("%s - %s", msg.Msg, string(m))
	}
	return w.sw.Crit(message)
}

// Err logs a message with severity LOG_ERR
func (w *bsd) Err(msg *Message) error {
	message := fmt.Sprintf("%s", msg.Msg)
	if msg.StructuredData != nil {
		m, err := json.Marshal(msg.StructuredData)
		if err != nil {
			return err
		}
		message = fmt.Sprintf("%s - %s", msg.Msg, string(m))
	}
	return w.sw.Err(message)
}

// Warning logs a message with severity LOG_WARNING
func (w *bsd) Warning(msg *Message) error {
	message := fmt.Sprintf("%s", msg.Msg)
	if msg.StructuredData != nil {
		m, err := json.Marshal(msg.StructuredData)
		if err != nil {
			return err
		}
		message = fmt.Sprintf("%s - %s", msg.Msg, string(m))
	}
	return w.sw.Warning(message)
}

// Notice logs a message with severity LOG_NOTICE
func (w *bsd) Notice(msg *Message) error {
	message := fmt.Sprintf("%s", msg.Msg)
	if msg.StructuredData != nil {
		m, err := json.Marshal(msg.StructuredData)
		if err != nil {
			return err
		}
		message = fmt.Sprintf("%s - %s", msg.Msg, string(m))
	}
	return w.sw.Notice(message)
}

// Info logs a message with severity LOG_INFO
func (w *bsd) Info(msg *Message) error {
	message := fmt.Sprintf("%s", msg.Msg)
	if msg.StructuredData != nil {
		m, err := json.Marshal(msg.StructuredData)
		if err != nil {
			return err
		}
		message = fmt.Sprintf("%s - %s", msg.Msg, string(m))
	}
	return w.sw.Info(message)
}

// Debug logs a message with severity LOG_DEBUG
func (w *bsd) Debug(msg *Message) error {
	message := fmt.Sprintf("%s", msg.Msg)
	if msg.StructuredData != nil {
		m, err := json.Marshal(msg.StructuredData)
		if err != nil {
			return err
		}
		message = fmt.Sprintf("%s - %s", msg.Msg, string(m))
	}
	return w.sw.Debug(message)
}
