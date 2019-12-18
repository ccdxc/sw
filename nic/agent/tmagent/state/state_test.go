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

	"github.com/golang/mock/gomock"
	"github.com/jeromer/syslogparser"
	"github.com/jeromer/syslogparser/rfc3164"
	"github.com/jeromer/syslogparser/rfc5424"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	tpmprotos "github.com/pensando/sw/nic/agent/protos/tpmprotos"
	"github.com/pensando/sw/nic/utils/ntranslate/metrics"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/syslog"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

func parseRfc3164(ch chan []byte) (syslogparser.LogParts, error) {
	select {
	case buff := <-ch:
		p := rfc3164.NewParser(buff)
		if err := p.Parse(); err != nil {
			log.Errorf("failed to create new Rfc3164 parser, %s", err)
			log.Errorf("received syslog [%v]", string(buff))
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
			log.Errorf("failed to create new Rfc5424 parser, %s", err)
			log.Errorf("received syslog [%v]", string(buff))
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
				log.Infof("udp syslog server %s ready", l.LocalAddr().String())
				n, _, err := l.ReadFrom(buff)
				if err != nil {
					for _, s := range []string{"closed network connection", "EOF"} {
						if strings.Contains(err.Error(), s) {
							log.Infof("read %s from udp socket", s)
							return
						}
					}
					log.Infof("error %s from udp socket", err)
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
				log.Errorf("failed to accept connection, %s", err)
				return
			}

			for {
				buff := make([]byte, 1024)
				log.Infof("tcp syslog server %s ready", l.Addr().String())
				n, err := conn.Read(buff)
				if err != nil {
					for _, s := range []string{"closed network connection", "EOF"} {
						if strings.Contains(err.Error(), s) {
							log.Infof("read %s from tcp socket", s)
							return
						}
					}
					log.Infof("error %s from tcp socket", err)
					continue
				}
				ch <- buff[:n]
			}
		}()
		return l.Addr().(*net.TCPAddr).String(), func() { l.Close() }, nil

	}
	return "", nop, fmt.Errorf("invalid protocol %s", proto)
}

func startNetagent() (*state.Nagent, *restapi.RestServer, error) {
	dp, err := datapath.NewHalDatapath(datapath.Kind("mock"))
	if err != nil {
		log.Errorf("Could not create HAL mock datapath. Error %v", err)
		return nil, nil, err
	}

	// Set tenant creation expectation
	if dp.Kind.String() == "mock" {
		dp.Hal.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
	}

	nagent, err := state.NewNetAgent(dp, "", nil)
	if err != nil {
		log.Errorf("Could not create net agent")
		return nil, nil, err
	}

	nRest, err := restapi.NewRestServer(nagent, nil, nil, "127.0.0.1:")
	if err != nil {
		log.Errorf("failed to create rest server, %s", err)
		return nil, nil, err
	}

	log.Printf("net-agent {%+v} instantiated", nRest.GetListenURL())

	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedNetwork",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
	}
	err = nagent.CreateNetwork(&nt)
	if err != nil {
		log.Errorf("Failed to create network. {%v}", nt)
		return nil, nil, err
	}

	for i := 1; i <= 7; i++ {
		tn := &netproto.Tenant{
			TypeMeta: api.TypeMeta{Kind: "Tenant"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    fmt.Sprintf("vpc-%d", i),
				Namespace: "default",
				Name:      fmt.Sprintf("vpc-%d", i),
			},
		}
		err = nagent.CreateTenant(tn)
		if err != nil {
			log.Errorf("Failed to create tenant. {%v}", tn)
			return nil, nil, err
		}

		vrf := &netproto.Vrf{
			TypeMeta: api.TypeMeta{Kind: "Vrf"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    fmt.Sprintf("vpc-%d", i),
				Namespace: "default",
				Name:      fmt.Sprintf("vpc-%d", i),
			},
		}
		err = nagent.CreateVrf(vrf)
		if err != nil {
			log.Errorf("Failed to create vrf. {%v}", i)
			return nil, nil, err
		}
	}

	return nagent, nRest, nil
}

func TestValidateLifIdToLifName(t *testing.T) {
	ag, rest, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer func() {
		ag.Stop()
		rest.Stop()
	}()
	metrics.Agenturl = rest.GetListenURL()
	intfname := metrics.GetLifName(1)
	Assert(t, intfname == "lif1", "invalid intfname received")
}

func TestValidateFwlogPolicy(t *testing.T) {
	ag, rest, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer func() {
		ag.Stop()
		rest.Stop()
	}()

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(rest.GetListenURL(), ":")[1])
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
			},
		}
	}

	ag, rest, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer func() {
		ag.Stop()
		rest.Stop()
	}()

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(rest.GetListenURL(), ":")[1])
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

		//match collectors
		m, ok := ps.getCollector(vrf)
		Assert(t, ok == true, "didn't get collector for vrf %+v", vrf)

		vrfs, _ := json.MarshalIndent(ag.ListVrf(), "", "   ")
		fmt.Println(string(vrfs))

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
	ag, rest, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer func() {
		ag.Stop()
		rest.Stop()
	}()

	nm, err := ag.ValidateVrf("default", "default", "default")
	AssertOk(t, err, "failed to find name space")

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(rest.GetListenURL(), ":")[1])
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

			val, ok := v.(*fwlogCollector)
			if !ok {
				return false
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
				SourceVrf: nm.Status.VrfID,
				DestVrf:   nm.Status.VrfID,
				Fwaction:  halproto.SecurityAction_SECURITY_RULE_ACTION_IMPLICIT_DENY,
				Sipv4:     srcIP,
				Dipv4:     destIP,
				Dport:     10000,
				IpProt:    20000,
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
					// rfc3164: map[priority:14 facility:1 severity:6 timestamp:2018-11-13 19:29:50 +0000 UTC
					// hostname:Ranjiths-MBP.pensando.io tag:mytag
					// content:{\"action\":\"SECURITY_RULE_ACTION_ALLOW\",\"dPort\":\"10000\",\"dest\":\"192.168.20.1\",\"direction\":\"0\",\"ipProt\":\"20000\",\"rule-id\":\"0\",\"src\":\"192.168.10.1\"}\n]"
					l, err := parseRfc3164(c)
					AssertOk(t, err, "failed to get %s syslog", k)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["content"]),
						strings.ToLower(strings.TrimPrefix(e.fwEvent.Fwaction.String(), "SECURITY_RULE_ACTION_"))), "failed to match, expected %s, got %+v", e.fwEvent.Fwaction, l)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["content"]), fmt.Sprintf("%d", e.fwEvent.Dport)), "failed to match, expected %s, got %+v", e.fwEvent.Dport, l)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["content"]), destIPStr), "failed to match, expected %s, got %+v", e.fwEvent.Dipv4, l)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["content"]), srcIPStr), "failed to match, expected %s, got %+v", e.fwEvent.Sipv4, l)

				}
			} else {
				for _, c := range v {
					l, err := parseRfc5424(c)
					AssertOk(t, err, "failed to get %s syslog", k)
					// rfc5424 map[priority:14 severity:6 msg_id:0 message:\n structured_data:[firewall@Pensando rule-id=\"0\" src=\"192.168.10.1\"
					// dest=\"192.168.20.1\" dPort=\"10000\" ipProt=\"20000\" action=\"SECURITY_RULE_ACTION_ALLOW\" direction=\"0\"] facility:1
					// version:1 timestamp:2018-11-13 19:29:50 -0800 PST hostname:Ranjiths-MBP.pensando.io app_name:- proc_id:28973]"
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["structured_data"]), strings.ToLower(strings.TrimPrefix(e.fwEvent.Fwaction.String(), "SECURITY_RULE_ACTION_"))), "failed to match, expected %s, got %+v", e.fwEvent.Fwaction, l)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["structured_data"]), fmt.Sprintf("%d", e.fwEvent.Dport)), "failed to match, expected %s, got %+v", e.fwEvent.Dport, l)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["structured_data"]), destIPStr), "failed to match, expected %s, got %+v", e.fwEvent.Dipv4, l)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["structured_data"]), srcIPStr), "failed to match, expected %s, got %+v", e.fwEvent.Sipv4, l)
				}
			}
		}
	}
}

func TestPolicyUpdate(t *testing.T) {
	ag, rest, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer func() {
		ag.Stop()
		rest.Stop()
	}()

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(rest.GetListenURL(), ":")[1])
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
			log.Info("Create")
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
				col, ok := val.(*fwlogCollector)
				Assert(t, ok == true, "failed to get collector from map")

				transport := strings.Split(target.Transport, "/")
				expCol := fwlogCollector{
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
	ag, rest, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer func() {
		ag.Stop()
		rest.Stop()
	}()

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(rest.GetListenURL(), ":")[1])
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
		ps.fwLogCollectors.Store(ip, &fwlogCollector{
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
			if val, ok := v.(*fwlogCollector); ok {
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
	ag, rest, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer func() {
		ag.Stop()
		rest.Stop()
	}()

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(rest.GetListenURL(), ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()

	err = ps.CreateFlowExportPolicy(context.Background(), nil)
	AssertOk(t, err, "CreateFlowExportPolicy failed")
	err = ps.UpdateFlowExportPolicy(context.Background(), nil)
	AssertOk(t, err, "UpdateFlowExportPolicy failed")
	err = ps.DeleteFlowExportPolicy(context.Background(), nil)
	AssertOk(t, err, "failed to create tp agent")
	_, err = ps.GetFlowExportPolicy(context.Background(), nil)
	AssertOk(t, err, "DeleteFlowExportPolicy failed")
	_, err = ps.GetFlowExportPolicy(context.Background(), nil)
	AssertOk(t, err, "DeleteFlowExportPolicy failed")
	_, err = ps.ListFlowExportPolicy(context.Background())
	AssertOk(t, err, "ListFlowExportPolicy failed")
}

func TestTsdbInit(t *testing.T) {
	ag, rest, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer func() {
		ag.Stop()
		rest.Stop()
	}()

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(rest.GetListenURL(), ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()

	err = ps.TsdbInit(t.Name(), &mock.ResolverClient{})
	AssertOk(t, err, "failed to init tsdb")
}

func TestFwlogInit(t *testing.T) {
	ag, rest, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer func() {
		ag.Stop()
		rest.Stop()
	}()

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	ps, err := NewTpAgent(ctx, strings.Split(rest.GetListenURL(), ":")[1])
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
	log.Infof("memsize=%d instances=%d", mSize, instCount)

	fd, err := syscall.Open(shmPath, syscall.O_RDWR|syscall.O_CREAT, 0666)
	if err != nil || fd < 0 {
		t.Fatalf("failed to create %s,%s", shmPath, err)
	}
	defer syscall.Close(fd)

	err = ps.FwlogInit(shmPath)
	AssertOk(t, err, "failed to init fwlog")
}
