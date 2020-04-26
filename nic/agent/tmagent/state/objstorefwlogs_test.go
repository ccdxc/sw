// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package state

import (
	"bytes"
	"compress/gzip"
	"context"
	"encoding/csv"
	"fmt"
	"net"
	"net/http"
	_ "os"
	_ "os/exec"
	"runtime"
	"strings"
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/gorilla/mux"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	_ "github.com/pensando/sw/nic/agent/dscagent"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	_ "github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/protos/tpmprotos"
	servicetypes "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/netutils"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

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

// TestProcessFWEventForObjStore tests the fwlog to objstore pipeline
func TestProcessFWEventForObjStore(t *testing.T) {
	// Start local minio server
	l, err := net.Listen("tcp", "127.0.0.1:")
	AssertOk(t, err, "failed listen")
	minioServer(l)
	url := l.Addr().(*net.TCPAddr).String()
	// url := "127.0.0.1:19001"
	// url := "127.0.0.1:9000"

	c := gomock.NewController(t)
	defer c.Finish()

	r := mock.New()
	retryOpt := objstore.WithConnectRetries(1)
	_, err = objstore.NewClient("ten1", "svc1", r, retryOpt)
	Assert(t, err != nil, "failed test client error ")

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
	AssertOk(t, err, "failed to add objstore sercvice")

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")

	err = ps.FwlogInit(FwlogIpcShm)
	AssertOk(t, err, "failed to init FwLog")

	testChannel := make(chan TestObject, 10000)
	err = ps.ObjStoreInit("1", r, time.Duration(1)*time.Second, testChannel)
	AssertOk(t, err, "failed to init objectstore")

	t.Run("TestDenyFilter", func(t *testing.T) {
		err = createFwlogPolicy(ctx, ps, "psmTarget", monitoring.FwlogFilter_FIREWALL_ACTION_DENY)
		AssertOk(t, err, "failed to create policy")

		// send logs
		generateLogs(t, ps, halproto.SecurityAction_SECURITY_RULE_ACTION_DENY, 1)
		testObject := <-testChannel
		verifyLog(t, testObject)

		err = deleteFwlogPolicy(ctx, ps, "psmTarget")
		AssertOk(t, err, "failed to create policy")
	})

	t.Run("TestAllowFilter", func(t *testing.T) {
		err = createFwlogPolicy(ctx, ps, "psmTarget", monitoring.FwlogFilter_FIREWALL_ACTION_ALLOW)
		AssertOk(t, err, "failed to create policy")

		// send logs
		generateLogs(t, ps, halproto.SecurityAction_SECURITY_RULE_ACTION_ALLOW, 1)
		testObject := <-testChannel
		verifyLog(t, testObject)

		err = deleteFwlogPolicy(ctx, ps, "psmTarget")
		AssertOk(t, err, "failed to create policy")
	})

	t.Run("TestAllFilter", func(t *testing.T) {
		err = createFwlogPolicy(ctx, ps, "psmTarget", monitoring.FwlogFilter_FIREWALL_ACTION_ALL)
		AssertOk(t, err, "failed to create policy")

		// send logs
		generateLogs(t, ps, halproto.SecurityAction_SECURITY_RULE_ACTION_ALLOW, 1)
		testObject := <-testChannel
		verifyLog(t, testObject)

		generateLogs(t, ps, halproto.SecurityAction_SECURITY_RULE_ACTION_DENY, 1)
		testObject = <-testChannel
		verifyLog(t, testObject)

		generateLogs(t, ps, halproto.SecurityAction_SECURITY_RULE_ACTION_REJECT, 1)
		testObject = <-testChannel
		verifyLog(t, testObject)

		err = deleteFwlogPolicy(ctx, ps, "psmTarget")
		AssertOk(t, err, "failed to create policy")
	})

	t.Run("TestNoneFilter", func(t *testing.T) {
		err = createFwlogPolicy(ctx, ps, "psmTarget", monitoring.FwlogFilter_FIREWALL_ACTION_NONE)
		AssertOk(t, err, "failed to create policy")

		// Since there will be no logs reported on test channel, it should time out.
		// send logs
		generateLogs(t, ps, halproto.SecurityAction_SECURITY_RULE_ACTION_ALLOW, 1)
		timedOut := false
		select {
		case <-time.After(time.Second):
			timedOut = true
		case <-testChannel:
			timedOut = false
		}
		Assert(t, timedOut, "received log even after None filter")

		generateLogs(t, ps, halproto.SecurityAction_SECURITY_RULE_ACTION_DENY, 1)
		timedOut = false
		select {
		case <-time.After(time.Second):
			timedOut = true
		case <-testChannel:
			timedOut = false
		}
		Assert(t, timedOut, "received log even after None filter")

		err = deleteFwlogPolicy(ctx, ps, "psmTarget")
		AssertOk(t, err, "failed to create policy")
	})

	// TestObjStoreErrors tests the error scenarios in the object store pipeline
	t.Run("TestObjStoreErrors", func(t *testing.T) {
		// Pass nil resolver
		err = ps.ObjStoreInit("1", nil, time.Duration(1)*time.Second, nil)
		Assert(t, err != nil, "failed to init objectstore")
	})

	// TestInternalUntriggeredFunctions tests the functions that are not triggered by the testcases written above
	t.Run("TestInternalUntriggeredFunctions", func(t *testing.T) {
		metric.addDrop()
		Assert(t, metric.fwlogDrops.Value() == 1, "fwlogDrops metric did not update")
		metric.addSuccess()
		Assert(t, metric.fwlogSuccess.Value() == 6, "fwlogSuccess metric did not update, current value", metric.fwlogSuccess.Value())
		metric.addRetries(5)
		Assert(t, metric.fwlogRetries.Get("5").String() == "1", "fwlogRetries metric did not update")
	})

	// Close the first ps
	ps.Close()

	// Init new ps for following tests
	ctx, cancel = context.WithCancel(context.Background())
	defer cancel()

	ps, err = NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()

	err = ps.FwlogInit(FwlogIpcShm)
	AssertOk(t, err, "failed to init FwLog")

	err = ps.ObjStoreInit("1", r, time.Duration(1)*time.Second, nil)
	AssertOk(t, err, "failed to init objectstore")

	// TestInternalUntriggeredFunctions tests the functions that are not triggered by the testcases written above
	t.Run("TestConnectivityLoss", func(t *testing.T) {
		// debug.SetGCPercent(-10)
		err = createFwlogPolicy(ctx, ps, "psmTarget", monitoring.FwlogFilter_FIREWALL_ACTION_ALL)
		AssertOk(t, err, "failed to create policy")
		memstats := runtime.MemStats{}
		runtime.ReadMemStats(&memstats)
		bHeapAlloc := memstats.HeapAlloc
		// fmt.Println("HeapStats beofre pushing logs", memstats.HeapAlloc, memstats.HeapInuse)
		// 100000 logs should not need more then 20 Mb.
		// 100000 * 200 bytes = 20Mb
		// 100000 logs will be able to hold 17mins worth of fwlogs at the sustained rate of 100/s
		numLogs := 100000
		maxLogLimitPerFile := 6000

		generateLogs(t, ps, halproto.SecurityAction_SECURITY_RULE_ACTION_ALLOW, numLogs)

		// Wait for all the pending items to get ingested into the pipeline
		// 17: 100000/6000
		AssertEventually(t, func() (bool, interface{}) {
			return metric.fwlogPendingItems.Value() == int64(numLogs/maxLogLimitPerFile), nil
		}, "syslog reconnect failed")

		memstats = runtime.MemStats{}
		runtime.ReadMemStats(&memstats)
		aHeapAlloc := memstats.HeapAlloc

		fmt.Println("Used heap", (aHeapAlloc-bHeapAlloc)/1000000)
		Assert(t, (aHeapAlloc-bHeapAlloc)/1000000 <= 50, "storing %d logs is taking more then 50Mb", numLogs)

		err = deleteFwlogPolicy(ctx, ps, "psmTarget")
		AssertOk(t, err, "failed to create policy")
	})

	// done := make(chan bool)
	// <-done
}

func verifyLog(t *testing.T, testObject TestObject) {
	Assert(t, testObject.ObjectName != "", "object name is empty")
	Assert(t, testObject.BucketName == "default.fwlogs", "bucket name is not correct")
	Assert(t, testObject.IndexBucketName == "meta-default.fwlogs", "index bucket name is not correct")
	verifyData(t, testObject.Data)
	Assert(t, testObject.Index != "", "index is empty")
	Assert(t, len(testObject.Meta) == 7, "object meta is empty %s", testObject.Meta)
	Assert(t, testObject.Meta["startts"] != "", "object meta's startts is empty")
	Assert(t, testObject.Meta["endts"] != "", "object meta's endts is empty")
	Assert(t, testObject.Meta["logcount"] == "1",
		"object meta's log count is not correct, expectecd 1, received %d", testObject.Meta["logcount"])
	Assert(t, testObject.Meta["nodeid"] == "1",
		"object meta's nodeid is not correct, expectecd 1, received %d", testObject.Meta["nodeid"])
	Assert(t, testObject.Meta["csvversion"] == "v1",
		"object meta's data csv version is not correct, expectecd v1, received %d", testObject.Meta["csvversion"])
	Assert(t, testObject.Meta["metaversion"] == "v1",
		"object meta's meta csv version is not correct, expectecd v1, received %d", testObject.Meta["metaversion"])

	// Check metrics
	Assert(t, metric.fwlogDrops.Value() == int64(0), "fwlog object got dropped")
	Assert(t, metric.fwlogSuccess.Value() != int64(0), "fwlog object success metric is 0")
}

func verifyData(t *testing.T, data bytes.Buffer) {
	reader := bytes.NewReader(data.Bytes())
	Assert(t, data.Len() != 0, "data is empty")
	zipReader, err := gzip.NewReader(reader)
	AssertOk(t, err, "error in unzipping data")

	rd := csv.NewReader(zipReader)
	AssertOk(t, err, "error in reading csv file")
	lines, err := rd.ReadAll()
	Assert(t, len(lines) == 2, "incorrect number of logs in the file")

	Assert(t, lines[1][0] == "65", "incorrect src vrf")
	Assert(t, lines[1][1] == "65", "incorrect dest vrf")
	Assert(t, lines[1][2] == "192.168.10.1", "incorrect source ip")
	Assert(t, lines[1][3] == "192.168.20.1", "incorrect dest ip")
	Assert(t, lines[1][16] == "32", "incorrect app id")
	Assert(t, lines[1][17] == "TFTP", "incorrect alg, %s", lines[1][17])
	Assert(t, lines[1][18] == "1", "incorrect count")
}

func createFwlogPolicy(ctx context.Context, ps *PolicyState, name string, filter monitoring.FwlogFilter) error {
	policy := &tpmprotos.FwlogPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "FwlogPolicy",
		},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec: monitoring.FwlogPolicySpec{
			Filter: []string{filter.String()},
			PSMTarget: &monitoring.PSMExportTarget{
				Enable: true,
			},
		},
	}
	return ps.CreateFwlogPolicy(ctx, policy)
}

func deleteFwlogPolicy(ctx context.Context, ps *PolicyState, name string) error {
	policy := &tpmprotos.FwlogPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "FwlogPolicy",
		},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
	}
	return ps.DeleteFwlogPolicy(ctx, policy)
}

func generateLogs(t *testing.T, ps *PolicyState, action halproto.SecurityAction, count int) {
	srcIPStr := "192.168.10.1"
	srcIP, err := netutils.IPv4ToUint32(srcIPStr)
	AssertOk(t, err, "failed to convert ip address")

	destIPStr := "192.168.20.1"
	destIP, err := netutils.IPv4ToUint32(destIPStr)
	AssertOk(t, err, "failed to convert ip address")

	// send logs
	for i := 0; i < count; i++ {
		fwEvent := &halproto.FWEvent{
			SourceVrf:       65,
			DestVrf:         65,
			Sipv4:           srcIP,
			Dipv4:           destIP,
			Dport:           10000,
			IpProt:          1,
			Fwaction:        action,
			Direction:       0,
			Alg:             halproto.ALGName_APP_SVC_TFTP,
			SessionId:       uint64(1000),
			ParentSessionId: uint64(1001),
			RuleId:          uint64(1000),
			Icmptype:        0,
			Icmpcode:        0,
			Icmpid:          0,
			AppId:           32,
		}

		ps.ProcessFWEvent(fwEvent, time.Now())
	}
}
