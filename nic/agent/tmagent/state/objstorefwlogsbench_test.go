package state_test

import (
	"context"
	"fmt"
	"net"
	"net/http"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/tpmprotos"
	"github.com/pensando/sw/nic/agent/tmagent/state"
	servicetypes "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/netutils"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver/mock"
)

type event struct {
	msgBsd     string
	msgRfc5424 string
	fwEvent    *halproto.FWEvent
}

const eventsPerSecond = 1300
const timeoutInSeconds = 180

func listBuckets(w http.ResponseWriter, r *http.Request) {
	resp := `{<?xml version="1.0" encoding="UTF-8"?>
<ListAllMyBucketsResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/"><Owner><ID>02d6176db174dc93cb1b899f7c6078f08654445fe8cf1b6ce98d8855f66bdbf4</ID><DisplayName></DisplayName></Owner><Buckets><Bucket><Name>tenant1-pktcap</Name><CreationDate>2018-06-05T00:44:14.880Z</CreationDate></Bucket><Bucket><Name>tenant1.pktcap</Name><CreationDate>2018-06-05T07:05:22.601Z</CreationDate></Bucket></Buckets></ListAllMyBucketsResult>} `
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(resp))
	w.Header().Set("Content-Type", "application/xml")
}

func minioServer(l net.Listener) {
	router := mux.NewRouter()
	router.HandleFunc(fmt.Sprintf("/"), listBuckets).Methods("GET")
	go http.Serve(l, router)
}

func SkipBenchmarkTmAgentFwLogs(b *testing.B) {
	fmt.Println("RUN *******")

	l, err := net.Listen("tcp", "127.0.0.1:")
	if err != nil {
		panic(err)
	}
	minioServer(l)
	defer l.Close()
	url := "127.0.0.1:9000"

	r := mock.New()
	retryOpt := objstore.WithConnectRetries(1)
	_, err = objstore.NewClient("ten1", "svc1", r, retryOpt)

	err = r.AddServiceInstance(&servicetypes.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.VosMinio,
		},
		Service: globals.VosMinio,
		URL:     url,
	})

	for n := 0; n < b.N; n++ {
		ctx, cancel := context.WithCancel(context.Background())
		defer cancel()

		ps, err := state.NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
		if err != nil {
			panic(err)
		}
		defer ps.Close()

		err = ps.FwlogInit(state.FwlogIpcShm)
		if err != nil {
			panic(err)
		}

		err = ps.ObjStoreInit("1", r, time.Duration(5)*time.Second, nil)
		if err != nil {
			panic(err)
		}

		err = createFwlogPolicy(ctx, ps)
		if err != nil {
			panic(err)
		}

		srcIPStr := "192.168.10.1"
		srcIP, err := netutils.IPv4ToUint32(srcIPStr)

		destIPStr := "192.168.20.1"
		destIP, err := netutils.IPv4ToUint32(destIPStr)

		wg := sync.WaitGroup{}
		wg.Add(1)
		go func() {
			defer wg.Done()
			j := 1
			for {
				j++
				if j == timeoutInSeconds {
					return
				}

				// send logs
				events := []event{}
				for i := 0; i < eventsPerSecond; i++ {
					events = append(events, event{
						fwEvent: &halproto.FWEvent{
							SourceVrf: 1000,
							DestVrf:   1001,
							Fwaction:  halproto.SecurityAction_SECURITY_RULE_ACTION_DENY,
							Sipv4:     srcIP,
							Dipv4:     destIP,
							Dport:     uint32(10000 + i),
							IpProt:    1,
							AppId:     32,
						},
					})
				}

				for _, e := range events {
					ps.ProcessFWEvent(e.fwEvent, time.Now())
				}

				time.Sleep(1 * time.Second)
			}
		}()

		wg.Wait()
	}
}

func createFwlogPolicy(ctx context.Context, ps *state.PolicyState) error {
	policy := &tpmprotos.FwlogPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "FwlogPolicy",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "policy2",
		},
		Spec: monitoring.FwlogPolicySpec{
			Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_REJECT.String()},
			PSMTarget: &monitoring.PSMExportTarget{
				Enable: true,
			},
		},
	}
	return ps.CreateFwlogPolicy(ctx, policy)
}
