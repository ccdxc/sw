package fwevent

import (
	"context"
	"fmt"
	"math/rand"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

// NewFwEventGen generates fwlogs
func NewFwEventGen(ctx context.Context, num int) chan *halproto.FWEvent {
	protKey := func() int32 {
		for k := range halproto.IPProtocol_name {
			if k != 0 {
				return k
			}
		}
		return 1 // TCP
	}

	fwActionKey := func() int32 {
		for k := range halproto.SecurityAction_name {
			if k != 0 {
				return k
			}
		}
		return 1 // ALLOW
	}
	evch := make(chan *halproto.FWEvent, 1)

	go func() {
		for i := 0; i < num; i++ {
			ev := &halproto.FWEvent{
				Flowaction: halproto.FlowLogEventType_FLOW_LOG_EVENT_TYPE_CREATE,
				Sipv4:      uint32(rand.Int31n(200) + rand.Int31n(200)<<8 + rand.Int31n(200)<<16 + rand.Int31n(200)<<24),
				Dipv4:      uint32(rand.Int31n(200) + rand.Int31n(200)<<8 + rand.Int31n(200)<<16 + rand.Int31n(200)<<24),
				Dport:      uint32(rand.Int31n(5000)),
				Sport:      uint32(rand.Int31n(5000)),
				IpProt:     halproto.IPProtocol(protKey()),
				Fwaction:   halproto.SecurityAction(fwActionKey()),
				Direction:  uint32(rand.Int31n(2) + 1),
				RuleId:     uint64(rand.Int63n(5000)),
				SessionId:  uint64(rand.Int63n(5000)),
			}

			if ev.IpProt == halproto.IPProtocol_IPPROTO_ICMP {
				ev.Icmpcode = uint32(rand.Int31n(16))
				ev.Icmptype = uint32(rand.Int31n(5))
				ev.Icmpid = uint32(rand.Int31n(5000))
			}
			select {
			case evch <- ev:
			case <-ctx.Done():
				break
			}
		}
		close(evch)

	}()

	return evch
}

// Cmd generates the command to trigger fw events in the specified naple
func Cmd(num int) string {
	return fmt.Sprintf("curl -s -X POST http://127.0.0.1:9257/fwlog?num=%d", num)
}
