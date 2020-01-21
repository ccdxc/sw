// Package apc for restarting host
//lint:file-ignore SA6004 Expect *requires* regexps
package apc

import (
	"fmt"
	"io"
	"regexp"
	"time"

        "github.com/pensando/sw/venice/utils/log"
	expect "github.com/pensando/goexpect"
	"github.com/pkg/errors"
)

const timeout = 30 * time.Second

// Host is the APC server structure
type Host struct {
	exp expect.Expecter
}

// Dial connects specified APC server
func Dial(ip, username, password string, output io.WriteCloser) (*Host, error) {
	exp, _, err := expect.Spawn(fmt.Sprintf("telnet %s", ip), -1,
		expect.Tee(output),
		expect.CheckDuration(50*time.Millisecond),
		expect.Verbose(true),
		expect.VerboseWriter(output))
	if err != nil {
		return nil, err
	}

	if _, _, err := exp.Expect(regexp.MustCompile("User Name : "), timeout); err != nil {
		return nil, errors.Wrap(err, "while expecting password")
	}

	if err := exp.Send(username + "\r"); err != nil {
		return nil, errors.Wrap(err, "while sending username")
	}
	if _, _, err := exp.Expect(regexp.MustCompile("Password  : "), timeout); err != nil {
		return nil, errors.Wrap(err, "while expecting password")
	}

	if err := exp.Send(password + "\r"); err != nil {
		return nil, errors.Wrap(err, "while sending password")
	}

	if _, _, err := exp.Expect(regexp.MustCompile(">"), timeout); err != nil {
		return nil, errors.Wrap(err, "while expecing password")
	}

	return &Host{
		exp: exp,
	}, nil
}

// Close closes the session
func (h *Host) Close() error {
	return h.exp.Close()
}

// Reset powers off and on the specified port
func (h *Host) Reset(port string) error {
	if err := h.exp.Send("olOff " + port + " \r\n"); err != nil {
		return errors.Wrapf(err, "while olOff %s", port)
	}
	if _, _, err := h.exp.Expect(regexp.MustCompile("E000: Success"), timeout); err != nil {
		return errors.Wrapf(err, "while expect olOff %s", port)
	}

	// Sleep 10 seconds to make sure command takes effect
	time.Sleep(10 * time.Second)

	if err := h.exp.Send("olOn " + port + " \r\n"); err != nil {
		return errors.Wrapf(err, "while olOn %s", port)
	}
	if _, _, err := h.exp.Expect(regexp.MustCompile("E000: Success"), timeout); err != nil {
		return errors.Wrapf(err, "while expect olOn %s", port)
	}
	return nil
}

// powers off the specified port
func (h *Host) PowerOff(port string) error {
        log.Infof("power off port %s",port)
	if err := h.exp.Send("olOff " + port + " \r\n"); err != nil {
		return errors.Wrapf(err, "while olOff %s", port)
	}
	if _, _, err := h.exp.Expect(regexp.MustCompile("E000: Success"), timeout); err != nil {
		return errors.Wrapf(err, "while expect olOff %s", port)
	}
	return nil
}

// powers on the specified port
func (h *Host) PowerOn(port string) error {
        log.Infof("power on port %s",port)
	if err := h.exp.Send("olOn " + port + " \r\n"); err != nil {
		return errors.Wrapf(err, "while olOn %s", port)
	}
	if _, _, err := h.exp.Expect(regexp.MustCompile("E000: Success"), timeout); err != nil {
		return errors.Wrapf(err, "while expect olOn %s", port)
	}
	return nil
}

