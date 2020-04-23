package state

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"reflect"
	"strings"
	"syscall"
	"testing"
	"time"

	"github.com/jeromer/syslogparser"
	"github.com/jeromer/syslogparser/rfc3164"
	"github.com/jeromer/syslogparser/rfc5424"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	fakehal "github.com/pensando/sw/nic/agent/cmd/fakehal/hal"
	"github.com/pensando/sw/nic/agent/dscagent"
	"github.com/pensando/sw/nic/agent/dscagent/infra"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/protos/tpmprotos"
	"github.com/pensando/sw/nic/utils/ntranslate/metrics"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/syslog"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

var mockAgent *dscagent.DSCAgent

func TestMain(m *testing.M) {
	// Start delphihub

	// Start Fake HAL
	var lis netutils.TestListenAddr
	if err := lis.GetAvailablePort(); err != nil {
		fmt.Printf("Test Setup Failed. Err: %v\n", err)
		os.Exit(1)
	}
	//if err := restLis.GetAvailablePort(); err != nil {
	//	fmt.Printf("Test Setup Failed. Err: %v", err)
	//	os.Exit(1)
	//}
	primaryDB, err := ioutil.TempFile("", "")
	if err != nil {
		fmt.Printf("Test Setup Failed. Err: %v\n", err)
		os.Exit(1)
	}

	secondaryDB, err := ioutil.TempFile("", "")
	if err != nil {
		fmt.Printf("Test Setup Failed. Err: %v\n", err)
		os.Exit(1)
	}
	fmt.Printf("Primary DB: %s | Backup DB: %s\n", primaryDB.Name(), secondaryDB.Name())

	if err := os.Setenv("HAL_GRPC_PORT", strings.Split(lis.ListenURL.String(), ":")[1]); err != nil {
		fmt.Printf("Test Setup Failed. Err: %v\n", err)
		os.Exit(1)
	}

	mockHal := fakehal.NewFakeHalServer(lis.ListenURL.String())

	logger := log.GetNewLogger(log.GetDefaultConfig("netagent_iris_test"))
	mockAgent, err = dscagent.NewDSCAgent(logger, "", "", "", types.DefaultAgentRestURL)
	if err != nil {
		fmt.Printf("Test Setup Failed. Err: %v\n", err)
		os.Exit(1)
	}
	time.Sleep(time.Second * 2)
	mockAgent.InfraAPI, err = infra.NewInfraAPI(primaryDB.Name(), secondaryDB.Name())
	if err != nil {
		fmt.Printf("Test Setup Failed. Err: %v\n", err)
		os.Exit(1)
	}

	// Create initial Vrfs for testing
	for i := 1; i <= 7; i++ {
		vrf := netproto.Vrf{
			TypeMeta: api.TypeMeta{Kind: "Vrf"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    fmt.Sprintf("vpc-%d", i),
				Namespace: "default",
				Name:      fmt.Sprintf("vpc-%d\n", i),
			},
		}
		_, err := mockAgent.PipelineAPI.HandleVrf(types.Create, vrf)
		if err != nil {
			fmt.Printf("Test Setup Failed. Err: %v\n", err)
			os.Exit(1)
		}

	}
	code := m.Run()
	mockAgent.Stop()
	mockHal.Stop()
	os.Remove(primaryDB.Name())
	os.Remove(secondaryDB.Name())
	os.Exit(code)

}

func parseRfc3164(ch chan []byte) (syslogparser.LogParts, error) {
	select {
	case buff := <-ch:
		p := rfc3164.NewParser(buff)
		if err := p.Parse(); err != nil {
			fmt.Printf("failed to create new Rfc3164 parser, %s\n", err)
			fmt.Printf("received syslog [%v]\n", string(buff))
			return nil, err
		}
		return p.Dump(), nil

	case <-time.After(time.Second):
		return nil, fmt.Errorf("timedout, no Rfc3164 syslog")

	}
}

func parseRfc5424(ch chan []byte) (syslogparser.LogParts, error) {
	select {
	case buff := <-ch:
		p := rfc5424.NewParser(buff)
		if err := p.Parse(); err != nil {
			fmt.Printf("failed to create new Rfc5424 parser, %s\n", err)
			fmt.Printf("received syslog [%v]\n", string(buff))
			return nil, err
		}
		return p.Dump(), nil

	case <-time.After(time.Second):
		return nil, fmt.Errorf("timedout, no Rfc5424 syslog")

	}
}

func startSyslogServer(proto string, ch chan []byte) (string, func(), error) {
	nop := func() {}
	if proto == "udp" {
		l, err := net.ListenPacket("udp", "127.0.0.1:0")
		if err != nil {
			return "", nop, err
		}

		// start reader goroutine
		go func() {
			for {
				buff := make([]byte, 1024)
				// block on read
				fmt.Printf("udp syslog server %s ready\n", l.LocalAddr().String())
				n, _, err := l.ReadFrom(buff)
				if err != nil {
					for _, s := range []string{"closed network connection", "EOF"} {
						if strings.Contains(err.Error(), s) {
							fmt.Printf("read %s from udp socket\n", s)
							return
						}
					}
					fmt.Printf("error %s from udp socket\n", err)
					continue
				}
				ch <- buff[:n]
			}
		}()

		return l.LocalAddr().(*net.UDPAddr).String(), func() { l.Close() }, nil

	} else if proto == "tcp" {
		l, err := net.Listen("tcp", "127.0.0.1:0")
		if err != nil {
			return "", nop, err
		}

		// start reader goroutine
		go func() {
			conn, err := l.Accept()
			if err != nil {
				fmt.Printf("failed to accept connection, %s\n", err)
				return
			}

			for {
				buff := make([]byte, 1024)
				fmt.Printf("tcp syslog server %s ready\n", l.Addr().String())
				n, err := conn.Read(buff)
				if err != nil {
					for _, s := range []string{"closed network connection", "EOF"} {
						if strings.Contains(err.Error(), s) {
							fmt.Printf("read %s from tcp socket\n", s)
							return
						}
					}
					fmt.Printf("error %s from tcp socket\n", err)
					continue
				}
				ch <- buff[:n]
			}
		}()
		return l.Addr().(*net.TCPAddr).String(), func() { l.Close() }, nil

	}
	return "", nop, fmt.Errorf("invalid protocol %s", proto)
}

func TestValidateLifIdToLifName(t *testing.T) {
	intfname := metrics.GetLifName(1)
	Assert(t, intfname == "lif-1", "invalid intfname received")
}

func TestValidateFwlogPolicy(t *testing.T) {

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	defer ps.Reset()
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()

	fwPolicy := []struct {
		name   string
		fail   bool
		policy *tpmprotos.FwlogPolicy
	}{
		{
			name: "no name",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "test.pensando.iox",
							Transport:   "tcp/15001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},
		{
			name: "invalid dns",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-dns",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "test.pensando.iox",
							Transport:   "tcp/15001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},
		{
			name: "too many targets",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "too-many-targets",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "localhost",
							Transport:   "tcp/15001",
						},
						{
							Destination: "192.168.100.1",
							Transport:   "udp/10001",
						},
						{
							Destination: "192.168.100.2",
							Transport:   "udp/10002",
						},
						{
							Destination: "192.168.100.1",
							Transport:   "tcp/10001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "duplicate targets",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "duplicate-targets",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},
		{
			name: "invalid destination",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "",
							Transport:   "tcp/10001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},
		{
			name: "invalid target",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "no port in target",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "no-port-in-target",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "192.168.100.11",
							Transport:   "tcp",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "invalid protocol, should be TCP/UDP",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-protocol",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "ip/10001",
						},
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "invalid port (abcd)",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-port",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/abcd",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "invalid port (65536)",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-port",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/65536",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "no targets",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "no-targets",
				},
				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},
		{
			name: "invalid override",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-override",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.12",
							Transport:   "tcp/10001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: "test-override",
					},
				},
			},
		},
		{
			name: "create good policy",
			fail: false,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "create-good-policy",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "localhost",
							Transport:   "tcp/15001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
					PSMTarget: &monitoring.PSMExportTarget{
						Enable: true,
					},
				},
			},
		},
	}

	for _, p := range fwPolicy {
		err = ps.CreateFwlogPolicy(ctx, p.policy)
		if p.fail {
			t.Logf("test [%s], error:%s", p.name, err)
			Assert(t, err != nil, "didn't fail to create fwlog export for [%s], policy: %+v", p.name, p.policy)
		} else {
			t.Logf("test [%s]", p.name)
			AssertOk(t, err, "failed to create fwlog export for [%s], policy: %+v", p.name, p.policy)
			//list
			l, err := ps.ListFwlogPolicy(ctx)
			AssertOk(t, err, "failed to list fwlog export for [%s], policy: %+v", p.name, p.policy)
			Assert(t, len(l) == 1, "policy count didnt match, got %d, expected 1 for [%s], policy %+v", len(l), p.name, p.policy)
			// get
			g, err := ps.GetFwlogPolicy(ctx, p.policy)
			AssertOk(t, err, "failed to get fwlog export for [%s], policy: %+v", p.name, p.policy)
			Assert(t, reflect.DeepEqual(g, p.policy) == true, "fwlog policy didnot match got %+v for [%s], policy: %+v", g, p.name, p.policy)

			err = ps.UpdateFwlogPolicy(ctx, p.policy)
			AssertOk(t, err, "failed to update fwlog export for [%s], policy: %+v", p.name, p.policy)

			err = ps.DeleteFwlogPolicy(ctx, p.policy)
			AssertOk(t, err, "failed to delete fwlog export for [%s], policy: %+v", p.name, p.policy)

			_, err = ps.Debug(&http.Request{})
			AssertOk(t, err, "failed to get debug")
		}
	}
}

func TestFwPolicyOps(t *testing.T) {
	genPolicy := map[int]*tpmprotos.FwlogPolicy{}

	for i := 0; i < 8; i++ {
		tenant := fmt.Sprintf("vpc-%d", i)
		if i == 0 {
			tenant = "default"
		}

		genPolicy[i] = &tpmprotos.FwlogPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "FwlogPolicy",
			},
			ObjectMeta: api.ObjectMeta{
				Namespace: globals.DefaultNamespace,
				Name:      tenant,
				Tenant:    tenant,
			},
			Spec: monitoring.FwlogPolicySpec{
				VrfName: tenant,
				Targets: []monitoring.ExportConfig{
					{
						Destination: fmt.Sprintf("192.168.100.%d", i+10),
						Transport:   fmt.Sprintf("tcp/%d", 10000+i),
					},
				},
				Format: monitoring.MonitoringExportFormat_vname[int32(i%2)],
				Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_REJECT.String()},
				Config: &monitoring.SyslogExportConfig{
					FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
				},
				PSMTarget: &monitoring.PSMExportTarget{
					Enable: true,
				},
			},
		}
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()

	// create
	for _, i := range genPolicy {
		err = ps.CreateFwlogPolicy(ctx, i)
		AssertOk(t, err, "failed to create fwlog policy %+v", i)

		// verify
		cmap := map[string]bool{}
		vrf, err := ps.getvrf(i.Tenant, i.Namespace, i.Spec.VrfName)
		AssertOk(t, err, "failed to get vrf %+v", i)

		// get all collectors
		for _, c := range i.Spec.Targets {
			key := ps.getCollectorKey(vrf, i, c)
			cmap[key] = true
		}
		if i.Spec.PSMTarget != nil && i.Spec.PSMTarget.Enable {
			key := ps.getPSMCollectorKey(vrf, i)
			cmap[key] = true
		}

		//match collectors
		m, ok := ps.getCollector(vrf)
		Assert(t, ok == true, "didn't get collector for vrf %+v", vrf)
		v := netproto.Vrf{TypeMeta: api.TypeMeta{Kind: "Vrf"}}
		vrfs, _ := mockAgent.PipelineAPI.HandleVrf(types.List, v)

		vrfJSON, _ := json.MarshalIndent(vrfs, "", "   ")
		fmt.Println(string(vrfJSON))

		for k := range m {
			_, ok := cmap[k]
			Assert(t, ok == true, "didn't find collector %+v", k)
			delete(cmap, k)
		}

		Assert(t, len(cmap) == 0, "didn't find collector %+v", cmap)
	}

	// delete
	for _, i := range genPolicy {
		err = ps.DeleteFwlogPolicy(ctx, i)
		AssertOk(t, err, "failed to delete fwlog policy %+v", i)

		// verify
		vrf, err := ps.getvrf(i.Tenant, i.Namespace, i.Spec.VrfName)
		AssertOk(t, err, "failed to get vrf %+v", i)
		c, ok := ps.getCollector(vrf)
		Assert(t, ok == false, "collectors exists after delete %+v", c)
	}

	// verify
	c := map[interface{}]bool{}
	ps.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {
		c[k] = true
		return true
	})
	Assert(t, len(c) == 0, "collectors exist after delete, %+v", c)
}

func TestProcessFWEvent(t *testing.T) {
	v := netproto.Vrf{
		TypeMeta: api.TypeMeta{
			Kind: "Vrf",
		},
		ObjectMeta: api.ObjectMeta{
			Name:      "default",
			Tenant:    "default",
			Namespace: "default",
		},
	}

	nm, err := mockAgent.PipelineAPI.HandleVrf(types.List, v)
	AssertOk(t, err, "failed to find vrf")

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()

	err = ps.TsdbInit(t.Name(), &mock.ResolverClient{})
	AssertOk(t, err, "failed to init tsdb ")

	tcpch1 := make(chan []byte, 1024)
	tcpAddr1, tcpl1, err := startSyslogServer("tcp", tcpch1)
	AssertOk(t, err, "failed to create tcp syslog server")
	defer tcpl1()

	tcpch2 := make(chan []byte, 1024)
	tcpAddr2, tcpl2, err := startSyslogServer("tcp", tcpch2)
	AssertOk(t, err, "failed to create tcp syslog server")
	defer tcpl2()

	tcpch3 := make(chan []byte, 1024)
	tcpAddr3, tcpl3, err := startSyslogServer("tcp", tcpch3)
	AssertOk(t, err, "failed to create tcp syslog server")
	defer tcpl3()

	udpch1 := make(chan []byte, 1024)
	udpAddr1, udpl1, err := startSyslogServer("udp", udpch1)
	AssertOk(t, err, "failed to create udp syslog server")
	defer udpl1()

	udpch2 := make(chan []byte, 1024)
	udpAddr2, udpl2, err := startSyslogServer("udp", udpch2)
	AssertOk(t, err, "failed to create udp syslog server")
	defer udpl2()

	fwPolicy1 := &tpmprotos.FwlogPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "FwlogPolicy",
		},
		ObjectMeta: api.ObjectMeta{
			Namespace: globals.DefaultNamespace,
			Name:      "policy1",
		},
		Spec: monitoring.FwlogPolicySpec{
			Targets: []monitoring.ExportConfig{
				{
					Destination: strings.Split(tcpAddr1, ":")[0],
					Transport:   fmt.Sprintf("tcp/%s", strings.Split(tcpAddr1, ":")[1]),
				},
				{
					Destination: strings.Split(udpAddr1, ":")[0],
					Transport:   fmt.Sprintf("udp/%s", strings.Split(udpAddr1, ":")[1]),
				},
			},
			Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
			Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
			Config: &monitoring.SyslogExportConfig{
				FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
			},
		},
	}
	fwPolicy2 := &tpmprotos.FwlogPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "FwlogPolicy",
		},
		ObjectMeta: api.ObjectMeta{
			Namespace: globals.DefaultNamespace,
			Name:      "policy2",
		},
		Spec: monitoring.FwlogPolicySpec{
			Targets: []monitoring.ExportConfig{
				{
					Destination: strings.Split(tcpAddr2, ":")[0],
					Transport:   fmt.Sprintf("tcp/%s", strings.Split(tcpAddr2, ":")[1]),
				},
				{
					Destination: strings.Split(udpAddr2, ":")[0],
					Transport:   fmt.Sprintf("udp/%s", strings.Split(udpAddr2, ":")[1]),
				},
			},
			Format: monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
			Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
			Config: &monitoring.SyslogExportConfig{
				FacilityOverride: monitoring.SyslogFacility_LOG_LOCAL0.String(),
			},
		},
	}
	fwPolicy3 := &tpmprotos.FwlogPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "FwlogPolicy",
		},
		ObjectMeta: api.ObjectMeta{
			Namespace: globals.DefaultNamespace,
			Name:      "policy3",
		},
		Spec: monitoring.FwlogPolicySpec{
			Targets: []monitoring.ExportConfig{
				{
					Destination: strings.Split(tcpAddr3, ":")[0],
					Transport:   fmt.Sprintf("tcp/%s", strings.Split(tcpAddr3, ":")[1]),
				},
			},
			Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
			Filter: []string{monitoring.FwlogFilter_FIREWALL_IMPLICIT_DENY.String()},
			Config: &monitoring.SyslogExportConfig{
				FacilityOverride: monitoring.SyslogFacility_LOG_LOCAL0.String(),
			},
		},
	}

	err = ps.CreateFwlogPolicy(ctx, fwPolicy1)
	AssertOk(t, err, "failed to create fwlog policy")

	err = ps.CreateFwlogPolicy(ctx, fwPolicy2)
	AssertOk(t, err, "failed to create fwlog policy")

	err = ps.CreateFwlogPolicy(ctx, fwPolicy3)
	AssertOk(t, err, "failed to create fwlog policy")

	srcIPStr := "192.168.10.1"
	srcIP, err := netutils.IPv4ToUint32(srcIPStr)
	AssertOk(t, err, "failed to convert ip address")

	destIPStr := "192.168.20.1"
	destIP, err := netutils.IPv4ToUint32(destIPStr)
	AssertOk(t, err, "failed to convert ip address")

	// wait to connect to syslog
	AssertEventually(t, func() (bool, interface{}) {
		kmap := map[string]bool{}

		ps.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {
			key, ok := k.(string)
			if !ok {
				return false
			}

			val, ok := v.(*syslogFwlogCollector)
			if !ok {
				return true
			}

			val.Lock()
			defer val.Unlock()

			if val.syslogFd == nil {
				// still connecting to syslog
				return false
			}

			kmap[key] = true
			return true
		})

		// total 5 syslog collectors are configured
		return len(kmap) == 5, kmap

	}, "failed to connect to syslog", "1s")

	// send logs
	events := []struct {
		msgBsd     string
		msgRfc5424 string
		fwEvent    *halproto.FWEvent
	}{
		{
			fwEvent: &halproto.FWEvent{
				SourceVrf: nm[0].Status.VrfID,
				DestVrf:   nm[0].Status.VrfID,
				Fwaction:  halproto.SecurityAction_SECURITY_RULE_ACTION_IMPLICIT_DENY,
				Sipv4:     srcIP,
				Dipv4:     destIP,
				Dport:     10000,
				IpProt:    20000,
				AppId:     32,
			},
		},
	}

	for _, e := range events {
		ps.ProcessFWEvent(e.fwEvent, time.Time{})

		// verify
		for k, v := range map[string][]chan []byte{
			"rfc3164": {tcpch1, tcpch3, udpch1},
			"rfc5424": {tcpch2, udpch2},
		} {
			if k == "rfc3164" {
				for _, c := range v {

					// rfc3164: [content:
					// [{"action":"implicit_deny","app-id":"32","destination-address":"192.168.20.1","destination-port":10000,
					// "direction":"","protocol":"20000","rule-id":0,"session-id":0,"session-state":"flow_create",
					// "source-address":"192.168.10.1","source-port":0,"timestamp":"0001-01-01T00:00:00Z"}]
					l, err := parseRfc3164(c)
					AssertOk(t, err, "failed to get %s syslog", k)
					m, ok := l["content"]
					Assert(t, ok, "failed to get message from syslog  %+v", l)
					Assert(t, strings.Contains(fmt.Sprintf("%s", m),
						strings.ToLower(strings.TrimPrefix(e.fwEvent.Fwaction.String(), "SECURITY_RULE_ACTION_"))),
						"failed to match, expected %s, got %+v", e.fwEvent.Fwaction, m)
					Assert(t, strings.Contains(fmt.Sprintf("%s", m), fmt.Sprintf("%d", e.fwEvent.Dport)),
						"failed to match, expected %s, got %+v", e.fwEvent.Dport, m)
					Assert(t, strings.Contains(fmt.Sprintf("%s", m), destIPStr),
						"failed to match, expected %s, got %+v", e.fwEvent.Dipv4, m)
					Assert(t, strings.Contains(fmt.Sprintf("%s", m), srcIPStr),
						"failed to match, expected %s, got %+v", e.fwEvent.Sipv4, m)
					Assert(t, strings.Contains(fmt.Sprintf("%s", m), "app-id"),
						"failed to match, expected app-id field, got %+v", m)
				}
			} else {
				for _, c := range v {
					l, err := parseRfc5424(c)
					AssertOk(t, err, "failed to get %s syslog", k)
					m, ok := l["message"]
					Assert(t, ok, "failed to get message from syslog  %+v", l)
					// rfc5424: [app_name:pen-tmagent app-id:32 facility:16 hostname:rchirakk-cluster-1
					// message:[{"action":"implicit_deny","destination-address":"192.168.20.1",
					// "destination-port":10000,"direction":"","protocol":"20000","rule-id":0,
					// "session-id":0,"session-state":"flow_create","source-address":"192.168.10.1",
					// "source-port":0,"timestamp":"0001-01-01T00:00:00Z"}]
					Assert(t, strings.Contains(fmt.Sprintf("%s", m),
						strings.ToLower(strings.TrimPrefix(e.fwEvent.Fwaction.String(), "SECURITY_RULE_ACTION_"))),
						"failed to match, expected %s, got %+v", e.fwEvent.Fwaction, m)
					Assert(t, strings.Contains(fmt.Sprintf("%s", m), fmt.Sprintf("%d", e.fwEvent.Dport)),
						"failed to match, expected %s, got %+v", e.fwEvent.Dport, m)
					Assert(t, strings.Contains(fmt.Sprintf("%s", m), destIPStr),
						"failed to match, expected %v , got %+v", e.fwEvent.Dipv4, m)
					Assert(t, strings.Contains(fmt.Sprintf("%s", m), srcIPStr),
						"failed to match, expected %s, got %+v", e.fwEvent.Sipv4, m)
					Assert(t, strings.Contains(fmt.Sprintf("%s", m), "app-id"),
						"failed to match, expected app-id field, got %+v", m)
				}
			}
		}
	}
}

func TestPolicyUpdate(t *testing.T) {

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()

	collList := []*tpmprotos.FwlogPolicyEvent{
		// add
		{
			EventType: api.EventType_CreateEvent,
			Policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "policy1",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.99.1",
							Transport:   "tcp/11001", // tcp to fail connect()
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		// add
		{
			EventType: api.EventType_CreateEvent,
			Policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "policy2",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.99.1",
							Transport:   "tcp/11001",
						},
						{
							Destination: "192.168.99.2",
							Transport:   "tcp/11002",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_DENY.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},
		// update
		{
			EventType: api.EventType_UpdateEvent,
			Policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "policy2",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.99.2",
							Transport:   "tcp/11002",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_REJECT.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
					PSMTarget: &monitoring.PSMExportTarget{
						Enable: true,
					},
				},
			},
		},
		// delete
		{
			EventType: api.EventType_DeleteEvent,
			Policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "policy1",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.99.1",
							Transport:   "tcp/11001", // tcp to fail connect()
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},
		// update: format and facility change
		{
			EventType: api.EventType_UpdateEvent,
			Policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "policy2",
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.99.1",
							Transport:   "tcp/11001",
						},
						{
							Destination: "192.168.99.2",
							Transport:   "tcp/11002",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALLOW.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_LOCAL0.String(),
					},
					PSMTarget: &monitoring.PSMExportTarget{
						Enable: false,
					},
				},
			},
		},
	}

	vrf, err := ps.getvrf(globals.DefaultTenant, globals.DefaultNamespace, globals.DefaultVrf)
	AssertOk(t, err, "failed to get vrf")

	for _, fwPolicyEvent := range collList {
		policySpec := fwPolicyEvent.Policy.Spec
		filter := ps.getFilter(policySpec.Filter)

		switch fwPolicyEvent.EventType {
		case api.EventType_CreateEvent:
			fmt.Printf("Create\n")
			err := ps.CreateFwlogPolicy(ctx, fwPolicyEvent.Policy)
			AssertOk(t, err, "failed to create policy %+v", fwPolicyEvent.Policy)
		case api.EventType_UpdateEvent:
			err := ps.UpdateFwlogPolicy(ctx, fwPolicyEvent.Policy)
			AssertOk(t, err, "failed to update policy %+v", fwPolicyEvent.Policy)
		case api.EventType_DeleteEvent:
			err := ps.DeleteFwlogPolicy(ctx, fwPolicyEvent.Policy)
			AssertOk(t, err, "failed to delete policy %+v", fwPolicyEvent.Policy)
		}

		// check collector
		switch fwPolicyEvent.EventType {
		case api.EventType_CreateEvent, api.EventType_UpdateEvent:
			for _, target := range policySpec.Targets {
				key := ps.getCollectorKey(vrf, fwPolicyEvent.Policy, target)
				val, ok := ps.fwLogCollectors.Load(key)
				Assert(t, ok == true, "failed to get key from map")
				col, ok := val.(*syslogFwlogCollector)
				Assert(t, ok == true, "failed to get collector from map")

				transport := strings.Split(target.Transport, "/")
				expCol := syslogFwlogCollector{
					vrf:         vrf,
					filter:      filter,
					format:      policySpec.Format,
					facility:    syslog.Priority(monitoring.SyslogFacility_vvalue[policySpec.Config.FacilityOverride]),
					destination: target.Destination,
					proto:       transport[0],
					port:        transport[1],
				}

				AssertEquals(t, col.vrf, expCol.vrf, "vrf didn't match")
				AssertEquals(t, col.filter, expCol.filter, "filter didn't match")
				AssertEquals(t, col.format, expCol.format, "format didn't match")
				AssertEquals(t, col.facility, expCol.facility, "facility didn't match")
				AssertEquals(t, col.destination, expCol.destination, "destination didn't match")
				AssertEquals(t, col.proto, expCol.proto, "proto didn't match")
				AssertEquals(t, col.port, expCol.port, "port didn't match")
			}
			if policySpec.PSMTarget != nil {
				key := ps.getPSMCollectorKey(vrf, fwPolicyEvent.Policy)
				val, ok := ps.fwLogCollectors.Load(key)
				if policySpec.PSMTarget.Enable {
					Assert(t, ok == true, "failed to get key from map")
					col, ok := val.(*psmFwlogCollector)
					Assert(t, ok == true, "failed to get collector from map")
					AssertEquals(t, col.vrf, vrf, "vrf didn't match")
					AssertEquals(t, col.filter, filter, "filter didn't match")
				} else {
					Assert(t, ok == false, "found stale key from map")
				}
			}
		case api.EventType_DeleteEvent:
			for _, target := range policySpec.Targets {
				key := ps.getCollectorKey(vrf, fwPolicyEvent.Policy, target)
				_, ok := ps.fwLogCollectors.Load(key)
				Assert(t, ok != true, "key was present in map when it should have been deleted")
			}
		}

		// check total collectors
		polCollector := map[string]bool{}
		pl, err := ps.ListFwlogPolicy(ctx)
		AssertOk(t, err, "failed to list policy ")
		for _, p := range pl {
			for _, c := range p.Spec.Targets {
				polCollector[ps.getCollectorKey(vrf, p, c)] = true
			}
			if p.Spec.PSMTarget != nil && p.Spec.PSMTarget.Enable {
				polCollector[ps.getPSMCollectorKey(vrf, p)] = true
			}
		}

		activeCollector := map[string]bool{}
		ps.fwLogCollectors.Range(func(k, v interface{}) bool {
			activeCollector[k.(string)] = true
			return true
		})

		AssertEquals(t, len(polCollector), len(activeCollector), "expected %d collectors got %d, policy:%+v active:+%v",
			len(polCollector), len(activeCollector), polCollector, activeCollector)

		Assert(t, reflect.DeepEqual(polCollector, activeCollector), "collectors didn't match", polCollector, activeCollector)
	}
}

func TestSyslogConnect(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()

	tcpch1 := make(chan []byte, 1024)
	tcpAddr, tcpl1, err := startSyslogServer("tcp", tcpch1)
	AssertOk(t, err, "failed to create tcp syslog server")
	defer tcpl1()

	udpch1 := make(chan []byte, 1024)
	udpAddr, udpl1, err := startSyslogServer("udp", udpch1)
	AssertOk(t, err, "failed to create udp syslog server")
	defer udpl1()

	for k, ip := range map[string]string{"tcp": tcpAddr, "udp": udpAddr} {
		ps.fwLogCollectors.Store(ip, &syslogFwlogCollector{
			vrf:         1,
			format:      monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
			destination: strings.Split(ip, ":")[0],
			port:        strings.Split(ip, ":")[1],
			proto:       k,
		})
	}
	err = ps.connectSyslog()
	AssertOk(t, err, "failed to start recconnect-syslog")
	time.Sleep(time.Second)

	AssertEventually(t, func() (bool, interface{}) {
		server := map[string]bool{}
		ps.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {
			if val, ok := v.(*syslogFwlogCollector); ok {
				val.Lock()
				if val.syslogFd == nil {
					server[k.(string)] = true
				}
				val.Unlock()
			}
			return true
		})
		return len(server) == 0, server

	}, "syslog reconnect failed")

}

func TestUnusedCb(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()
}

func TestTsdbInit(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()

	err = ps.TsdbInit(t.Name(), &mock.ResolverClient{})
	AssertOk(t, err, "failed to init tsdb")
}

func TestFwlogInit(t *testing.T) {

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()

	err = ps.TsdbInit(t.Name(), &mock.ResolverClient{})
	AssertOk(t, err, "failed to init tsdb")
	defer tsdb.Cleanup()

	tmpFd, err := ioutil.TempFile("/tmp", t.Name())
	AssertOk(t, err, "failed to create temp file")
	defer os.Remove(tmpFd.Name()) // clean up
	shmPath := tmpFd.Name()

	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	fmt.Printf("memsize=%d instances=%d\n", mSize, instCount)

	fd, err := syscall.Open(shmPath, syscall.O_RDWR|syscall.O_CREAT, 0666)
	if err != nil || fd < 0 {
		t.Fatalf("failed to create %s,%s", shmPath, err)
	}
	defer syscall.Close(fd)

	err = ps.FwlogInit(shmPath)
	AssertOk(t, err, "failed to init fwlog")
}

func TestSyslogJson(t *testing.T) {
	fwevent := fwevent{
		FWEvent: &halproto.FWEvent{},
	}
	s := fwevent.String()
	fmt.Printf("%v\n", s)
	m := []map[string]interface{}{}
	err := json.Unmarshal([]byte(s), &m)
	AssertOk(t, err, "invalid json")
}
