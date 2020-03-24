package fwevent

import (
	"context"
	"fmt"
	"math/rand"
	"time"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
)

// NewFwEventGen generates fwlogs
func NewFwEventGen(ctx context.Context, num int, vrf uint64) chan *halproto.FWEvent {
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
				SourceVrf:  uint64(rand.Int31n(100)),
				Flowaction: halproto.FlowLogEventType(i % 2),
				Sipv4:      uint32(rand.Int31n(200) + rand.Int31n(200)<<8 + rand.Int31n(200)<<16 + rand.Int31n(200)<<24),
				Dipv4:      uint32(rand.Int31n(200) + rand.Int31n(200)<<8 + rand.Int31n(200)<<16 + rand.Int31n(200)<<24),
				Dport:      uint32(rand.Int31n(5000)),
				Sport:      uint32(rand.Int31n(5000)),
				IpProt:     halproto.IPProtocol(protKey()),
				Fwaction:   halproto.SecurityAction(fwActionKey()),
				Direction:  uint32(rand.Int31n(2) + 1),
				RuleId:     uint64(rand.Int63n(5000)),
				Timestamp:  time.Now().UnixNano(),
				SessionId:  uint64(i + 1),
				AppId:      uint32(rand.Int31n(1000)),
			}

			// set vrf
			if vrf != 0 {
				ev.SourceVrf = vrf
				ev.DestVrf = vrf
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
func Cmd(num int, vrf int) string {
	return fmt.Sprintf("curl -s -X POST http://127.0.0.1:9257/fwlog?\"num=%d&vrf=%d\"", num, vrf)
}
