package server

import (
	"context"
	"fmt"
	"net"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/ipfix"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestServer(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute*5)
	defer cancel()

	addr, ch, err := NewServer(ctx, "127.0.0.1:")
	AssertOk(t, err, "failed to start server")

	// send templates
	conn, err := net.Dial("udp", addr)
	AssertOk(t, err, "failed to start client")
	defer conn.Close()

	tmplt, err := ipfix.CreateTemplateMsg()
	AssertOk(t, err, "failed to create template")

	go func() {
		if _, err := conn.Write(tmplt); err != nil {
			t.Errorf("failed to send template, %v", err)
		}
	}()

	select {
	case m, ok := <-ch:
		hdr := m.Header
		Assert(t, ok, "failed to read from ipfix channel")
		Assert(t, len(m.TemplateRecords) == 3, "expected 3 templates, got %v", len(m.TemplateRecords))
		Assert(t, hdr.Version == 0x0a, "invalid version %v", hdr.Version)
		Assert(t, hdr.SequenceNumber == 0, "invalid sequence number %v", hdr.SequenceNumber)
		Assert(t, hdr.DomainID == 0, "invalid domain id %v", hdr.DomainID)
		Assert(t, hdr.Length == 464, "invalid length %v", hdr.Length)

		for i := range m.TemplateRecords {
			r := m.TemplateRecords[i]
			Assert(t, int(r.TemplateID) == 257+i, "invalid template id %v", r.TemplateID)
		}

	case <-time.After(time.Second * 10):
		AssertOk(t, fmt.Errorf("timed-out"), "failed to receive templates")

	}
}

func TestInvalidTemplates(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute*5)

	_, _, err := NewServer(ctx, "127.0.0.1:")
	AssertOk(t, err, "failed to start server")
	cancel()

	ctx, cancel = context.WithTimeout(context.Background(), time.Minute*5)
	defer cancel()
	addr, ch, err := NewServer(ctx, "127.0.0.1:")
	AssertOk(t, err, "failed to start server")

	go func() {
		conn, err := net.Dial("udp", addr)
		AssertOk(t, err, "failed to start client")
		defer conn.Close()
		b := make([]byte, 100)
		if _, err := conn.Write(b); err != nil {
			t.Errorf("failed to send template, %v", err)
		}
	}()

	select {
	case m := <-ch:
		Assert(t, true, "received invalid template %v", m)
	case <-time.After(time.Second * 5):
		t.Logf("timed-out")
	}
}
