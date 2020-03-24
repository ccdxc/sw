// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"math/rand"
	"os"
	"time"

	halproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/venice/utils/log"
)

const fwlogIpcShm = "/fwlog_ipc_shm"

var numEntries = flag.Int("num", 1000, "Number of firewall entries")
var rateps = flag.Int("rate", 100, "Rate per second")
var metrics = flag.Bool("metrics", false, "Initialize metrics")

func main() {
	flag.Parse()

	if *rateps < 10 {
		fmt.Printf("Rate cant be below 10\n")
		os.Exit(1)
	}
	if *numEntries < *rateps {
		fmt.Printf("num entries(%d) cant be less than rate(%d)\n", *numEntries, *rateps)
		os.Exit(1)
	}

	if *metrics {
		metricsInit()
	}

	fmt.Printf("Generating %d fwlog entries at rate %d per second\n", *numEntries, *rateps)
	err := fwlogGen(fwlogIpcShm, *numEntries, *rateps)
	if err != nil {
		fmt.Printf("Error generating logs: %v\n", err)
	}
}

func fwlogGen(fwlogShm string, numEntries, rateps int) error {
	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
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

	shm, err := ipc.NewSharedMem(mSize, instCount, fwlogShm)
	if err != nil {
		return fmt.Errorf("failed to init fwlog, %s", err)
	}

	ipcList := make([]*ipc.IPC, instCount)
	for ix := 0; ix < instCount; ix++ {
		ipcList[ix] = shm.IPCInstance()
	}

	sentCount := 0
	for {
		nEntries := rateps / 10
		for idx := 0; idx < nEntries; idx++ {
			for _, fd := range ipcList {
				ev := &halproto.FWEvent{
					SourceVrf: uint64(1),
					DestVrf:   uint64(1),
					Sipv4:     uint32(rand.Int31n(200) + rand.Int31n(200)<<8 + rand.Int31n(200)<<16 + rand.Int31n(200)<<24),
					Dipv4:     uint32(192 + 168<<8 + rand.Int31n(200)<<16 + rand.Int31n(200)<<24),
					Dport:     uint32(rand.Int31n(4096)),
					Sport:     uint32(rand.Int31n(5000)),
					IpProt:    halproto.IPProtocol(protKey()),
					Fwaction:  halproto.SecurityAction(fwActionKey()),
					Direction: uint32(rand.Int31n(2) + 1),
					RuleId:    uint64(rand.Int63n(5000)),
					SessionId: uint64(rand.Int63n(5000)),
					AppId:     uint32(rand.Int31n(5000)),
				}

				if ev.IpProt == halproto.IPProtocol_IPPROTO_ICMP {
					ev.Icmpcode = uint32(rand.Int31n(16))
					ev.Icmptype = uint32(rand.Int31n(5))
					ev.Icmpid = uint32(rand.Int31n(5000))
				}
				if err := fd.Write(ev); err != nil {
					log.Errorf("failed to write fwlog, %s", err)
				}

			}
		}
		sentCount += nEntries
		if sentCount >= numEntries {
			break
		}
		// wait for 50ms after each write
		time.Sleep(time.Millisecond * 100)
	}

	return nil
}
