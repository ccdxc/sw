package main

import (
	"flag"
	"fmt"
	"net/http"
	"strconv"
	"time"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/nic/agent/tmagent/state/fwgen/fwevent"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

type fwgen struct {
	evch       chan *halproto.FWEvent
	numClients chan struct{}
	ipcList    []*ipc.IPC
}

func main() {
	var listenURL = flag.String("listen-url", ":9257", "-listen-url :9257")
	flag.Parse()

	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	shm, err := ipc.NewSharedMem(mSize, instCount, "/fwlog_ipc_shm")
	if err != nil {
		fmt.Println(err)
		return
	}
	fg := &fwgen{
		numClients: make(chan struct{}, 1),
		ipcList:    make([]*ipc.IPC, instCount),
	}

	for ix := 0; ix < instCount; ix++ {
		fg.ipcList[ix] = shm.IPCInstance()
	}

	r := mux.NewRouter()
	r.HandleFunc("/fwlog", netutils.MakeHTTPHandler(fg.numLogsHandler)).Methods("POST")
	log.Fatal(http.ListenAndServe(*listenURL, r))
}

func (f *fwgen) numLogsHandler(r *http.Request) (interface{}, error) {

	select {
	case f.numClients <- struct{}{}:
		defer func() { <-f.numClients }()

		numStr := r.URL.Query().Get("num")
		numLogs, err := strconv.Atoi(numStr)
		if err != nil {
			return nil, fmt.Errorf("invalid num-logs in %s, %s", numStr, err)
		}
		vrf := r.URL.Query().Get("vrf")
		vrfid, _ := strconv.Atoi(vrf)

		ch := fwevent.NewFwEventGen(r.Context(), numLogs, uint64(vrfid))
		idx := 0
		for ev := range ch {
			if err := f.ipcList[idx].Write(ev); err != nil {
				fmt.Printf("[%v]stall: %s", time.Now(), err)
			}
			idx = (idx + 1) % len(f.ipcList)
			time.Sleep(time.Millisecond)
		}

	default:
		return nil, fmt.Errorf("try again later")
	}

	return nil, nil
}
