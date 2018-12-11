package state

import (
	"context"
	"fmt"
	"net"
	"net/http"
	"reflect"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/gorilla/mux"
	"github.com/jeromer/syslogparser"
	"github.com/jeromer/syslogparser/rfc3164"
	"github.com/jeromer/syslogparser/rfc5424"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/ctrler/tpm/rpcserver/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	tsdb "github.com/pensando/sw/venice/utils/ntsdb"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/syslog"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// to mock netagent
type agentDb struct {
	vrfDb map[string]uint64
}

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

func (ag *agentDb) vrfHandler(r *http.Request) (interface{}, error) {

	tenant, _ := mux.Vars(r)["tenant"]
	namespace, _ := mux.Vars(r)["namespace"]

	log.Infof("received %s/%s db:%+v", tenant, namespace, ag.vrfDb)

	key := tenant + ":" + namespace
	vrf, ok := ag.vrfDb[key]
	if !ok {
		return nil, fmt.Errorf("no matching vrf")
	}

	log.Infof("returned vrf: %v", vrf)

	ns := &netproto.Namespace{
		Status: netproto.NamespaceStatus{
			NamespaceID: vrf,
		},
	}

	return ns, nil
}

func startNetagent() (net.Listener, error) {
	agDb := &agentDb{
		vrfDb: map[string]uint64{
			fmt.Sprintf("%s:%s", globals.DefaultTenant, globals.DefaultNamespace): 1,
			"vpc-1:default": 2,
			"vpc-2:default": 3,
			"vpc-3:default": 4,
			"vpc-4:default": 5,
			"vpc-5:default": 6,
			"vpc-6:default": 7,
			"vpc-7:default": 8,
		},
	}

	l, err := net.Listen("tcp", "127.0.0.1:")
	if err != nil {
		return nil, err
	}

	r := mux.NewRouter()
	r.HandleFunc("/api/namespace/{tenant}/{namespace}", netutils.MakeHTTPHandler(netutils.RestAPIFunc(agDb.vrfHandler))).Methods("GET")
	go http.Serve(l, r)

	log.Infof("mock netagent started %s", l.Addr().(*net.TCPAddr).String())
	return l, nil
}

func TestValidateFwLogPolicy(t *testing.T) {
	l, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer l.Close()

	/// init tsdb
	opt := &tsdb.Opts{
		ClientName:     "netagent-007",
		ResolverClient: &mock.ResolverClient{},
	}
	ctx, cancel := context.WithCancel(context.Background())

	tsdbCtx, tsdbCancel := context.WithCancel(ctx)
	tsdb.Init(tsdbCtx, opt)
	tsdbCancel() //abort tsdb transmit

	ps, err := NewTpAgent(ctx, strings.Split(l.Addr().(*net.TCPAddr).String(), ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()
	defer cancel()

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
					Kind: "fwLogPolicy",
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
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},
		{
			name: "invalid dns",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
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
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},
		{
			name: "too many collectors",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "registry.test.pensando.io",
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
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},

		{
			name: "duplicate collectors",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "registry.test.pensando.io",
							Transport:   "tcp/15001",
						},
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},
		{
			name: "invalid destination",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
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
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},
		{
			name: "invalid target",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},

		{
			name: "no port in collector",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
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
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},

		{
			name: "invalid proto",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
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
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},

		{
			name: "invalid port",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
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
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},

		{
			name: "invalid port",
			fail: true,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
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
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},
		{
			name: "create policy",
			fail: false,
			policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "registry.test.pensando.io",
							Transport:   "tcp/15001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: "FWLOG-RFC3164",
					},
				},
			},
		},
	}

	for _, p := range fwPolicy {
		err = ps.CreateFwLogPolicy(ctx, p.policy)
		if p.fail {
			t.Logf("test [%s], error:%s", p.name, err)
			Assert(t, err != nil, "didn't fail to create fwlog export for [%s], policy: %+v", p.name, p.policy)
		} else {
			AssertOk(t, err, "failed to create fwlog export for [%s], policy: %+v", p.name, p.policy)
			//list
			l, err := ps.ListFwLogPolicy(ctx)
			AssertOk(t, err, "failed to list fwlog export for [%s], policy: %+v", p.name, p.policy)
			Assert(t, len(l) == 1, "policy count didnt match, got %d, expected 1 for [%s], policy %+v", len(l), p.name, p.policy)
			// get
			g, err := ps.GetFwLogPolicy(ctx, p.policy)
			AssertOk(t, err, "failed to get fwlog export for [%s], policy: %+v", p.name, p.policy)
			Assert(t, reflect.DeepEqual(g, p.policy) == true, "fwlog policy didnot match got %+v for [%s], policy: %+v", g, p.name, p.policy)

			err = ps.UpdateFwLogPolicy(ctx, p.policy)
			AssertOk(t, err, "failed to update fwlog export for [%s], policy: %+v", p.name, p.policy)

			err = ps.DeleteFwLogPolicy(ctx, p.policy)
			AssertOk(t, err, "failed to delete fwlog export for [%s], policy: %+v", p.name, p.policy)

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
				Kind: "fwLogPolicy",
			},
			ObjectMeta: api.ObjectMeta{
				Namespace: globals.DefaultNamespace,
				Name:      tenant,
				Tenant:    tenant,
			},
			Spec: monitoring.FwlogPolicySpec{
				Targets: []monitoring.ExportConfig{
					{
						Destination: fmt.Sprintf("192.168.100.%d", i+10),
						Transport:   fmt.Sprintf("tcp/%d", 10000+i),
					},
				},
				Format: monitoring.MonitoringExportFormat_name[int32(i%2)],
				Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_REJECT.String()},
				Config: &monitoring.SyslogExportConfig{
					FacilityOverride: "FWLOG-RFC3164",
				},
			},
		}
	}

	l, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer l.Close()

	/// init tsdb
	opt := &tsdb.Opts{
		ClientName:     "netagent-007",
		ResolverClient: &mock.ResolverClient{},
	}
	ctx, cancel := context.WithCancel(context.Background())

	tsdbCtx, tsdbCancel := context.WithCancel(ctx)
	tsdb.Init(tsdbCtx, opt)
	tsdbCancel() //abort tsdb transmit

	ps, err := NewTpAgent(ctx, strings.Split(l.Addr().(*net.TCPAddr).String(), ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()
	defer cancel()

	// create
	for _, i := range genPolicy {
		err = ps.CreateFwLogPolicy(ctx, i)
		AssertOk(t, err, "failed to create fwlog policy %+v", i)

		// verify
		cmap := map[string]bool{}
		vrf, err := ps.getvrf(i.Tenant, i.Namespace)
		AssertOk(t, err, "failed to get vrf %+v", i)

		// get all collectors
		for _, c := range i.Spec.Targets {
			key := ps.getCollectorKey(vrf, i, c)
			cmap[key] = true
		}

		//match collectors
		m, ok := ps.getCollector(vrf)
		Assert(t, ok == true, "didn't get collector for vrf %+v", vrf)

		for k := range m {
			_, ok := cmap[k]
			Assert(t, ok == true, "didn't find collector %+v", k)
			delete(cmap, k)
		}

		Assert(t, len(cmap) == 0, "didn't find collector %+v", cmap)
	}

	// delete
	for _, i := range genPolicy {
		err = ps.DeleteFwLogPolicy(ctx, i)
		AssertOk(t, err, "failed to delete fwlog policy %+v", i)

		// verify
		vrf, err := ps.getvrf(i.Tenant, i.Namespace)
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
	l, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer l.Close()

	/// init tsdb
	opt := &tsdb.Opts{
		ClientName:     "netagent-007",
		ResolverClient: &mock.ResolverClient{},
	}
	ctx, cancel := context.WithCancel(context.Background())

	tsdbCtx, tsdbCancel := context.WithCancel(ctx)
	tsdb.Init(tsdbCtx, opt)
	tsdbCancel() //abort tsdb transmit

	ps, err := NewTpAgent(ctx, strings.Split(l.Addr().(*net.TCPAddr).String(), ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()
	defer cancel()

	tcpch1 := make(chan []byte, 1024)
	tcpAddr1, tcpl1, err := startSyslogServer("tcp", tcpch1)
	AssertOk(t, err, "failed to create tcp syslog server")
	defer tcpl1()

	tcpch2 := make(chan []byte, 1024)
	tcpAddr2, tcpl2, err := startSyslogServer("tcp", tcpch2)
	AssertOk(t, err, "failed to create tcp syslog server")
	defer tcpl2()

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
			Kind: "fwLogPolicy",
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
				FacilityOverride: "FWLOG-RFC3164",
			},
		},
	}
	fwPolicy2 := &tpmprotos.FwlogPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "fwLogPolicy",
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
				FacilityOverride: "FWLOG-RFC5424",
			},
		},
	}

	err = ps.CreateFwLogPolicy(ctx, fwPolicy1)
	AssertOk(t, err, "failed to create fwlog policy")
	err = ps.CreateFwLogPolicy(ctx, fwPolicy2)
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

			if val.syslogFd == nil {
				// still connecting to syslog
				return false
			}

			kmap[key] = true
			return true
		})

		// total 4 syslog collectors are configured
		return len(kmap) == 4, kmap

	}, "failed to connect to syslog", "1s")

	// send logs
	events := []struct {
		msgBsd     string
		msgRfc5424 string
		fwEvent    *halproto.FWEvent
	}{
		{
			fwEvent: &halproto.FWEvent{
				SourceVrf: 1,
				DestVrf:   1,
				Fwaction:  halproto.SecurityAction_SECURITY_RULE_ACTION_ALLOW,
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
			"rfc3164": {tcpch1, udpch1},
			"rfc5424": {tcpch2, udpch2},
		} {
			if k == "rfc3164" {
				for _, c := range v {
					// rfc3164: map[priority:14 facility:1 severity:6 timestamp:2018-11-13 19:29:50 +0000 UTC
					// hostname:Ranjiths-MBP.pensando.io tag:mytag
					// content:{\"action\":\"SECURITY_RULE_ACTION_ALLOW\",\"dPort\":\"10000\",\"dest\":\"192.168.20.1\",\"direction\":\"0\",\"ipProt\":\"20000\",\"rule-id\":\"0\",\"src\":\"192.168.10.1\"}\n]"
					l, err := parseRfc3164(c)
					AssertOk(t, err, "failed to get %s syslog", k)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["content"]), e.fwEvent.Fwaction.String()), "failed to match, expected %s, got %+v", e.fwEvent.Fwaction, l)
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
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["structured_data"]), e.fwEvent.Fwaction.String()), "failed to match, expected %s, got %+v", e.fwEvent.Fwaction, l)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["structured_data"]), fmt.Sprintf("%d", e.fwEvent.Dport)), "failed to match, expected %s, got %+v", e.fwEvent.Dport, l)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["structured_data"]), destIPStr), "failed to match, expected %s, got %+v", e.fwEvent.Dipv4, l)
					Assert(t, strings.Contains(fmt.Sprintf("%s", l["structured_data"]), srcIPStr), "failed to match, expected %s, got %+v", e.fwEvent.Sipv4, l)
				}
			}
		}
	}
}

func TestPolicyUpdate(t *testing.T) {
	l, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer l.Close()

	/// init tsdb
	opt := &tsdb.Opts{
		ClientName:     "netagent-007",
		ResolverClient: &mock.ResolverClient{},
	}
	ctx, cancel := context.WithCancel(context.Background())

	tsdbCtx, tsdbCancel := context.WithCancel(ctx)
	tsdb.Init(tsdbCtx, opt)
	tsdbCancel() //abort tsdb transmit

	ps, err := NewTpAgent(ctx, strings.Split(l.Addr().(*net.TCPAddr).String(), ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()
	defer cancel()

	collList := []*tpmprotos.FwlogPolicyEvent{
		// add
		&tpmprotos.FwlogPolicyEvent{
			EventType: api.EventType_CreateEvent,
			Policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
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
						FacilityOverride: "test",
					},
				},
			},
		},

		// add
		&tpmprotos.FwlogPolicyEvent{
			EventType: api.EventType_CreateEvent,
			Policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
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
						FacilityOverride: "test",
					},
				},
			},
		},
		// update
		&tpmprotos.FwlogPolicyEvent{
			EventType: api.EventType_UpdateEvent,
			Policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
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
						FacilityOverride: "test",
					},
				},
			},
		},
		// delete
		&tpmprotos.FwlogPolicyEvent{
			EventType: api.EventType_DeleteEvent,
			Policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
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
						FacilityOverride: "test",
					},
				},
			},
		},
		// update: format and facility change
		&tpmprotos.FwlogPolicyEvent{
			EventType: api.EventType_UpdateEvent,
			Policy: &tpmprotos.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
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
						FacilityOverride: "test-changed",
					},
				},
			},
		},
	}

	vrf, err := ps.getvrf(globals.DefaultTenant, globals.DefaultNamespace)
	AssertOk(t, err, "failed to get vrf")

	for _, fwPolicyEvent := range collList {
		policySpec := fwPolicyEvent.Policy.Spec
		filter := ps.getFilter(policySpec.Filter)

		switch fwPolicyEvent.EventType {
		case api.EventType_CreateEvent:
			log.Info("Create")
			err := ps.CreateFwLogPolicy(ctx, fwPolicyEvent.Policy)
			AssertOk(t, err, "failed to create policy %+v", fwPolicyEvent.Policy)
		case api.EventType_UpdateEvent:
			err := ps.UpdateFwLogPolicy(ctx, fwPolicyEvent.Policy)
			AssertOk(t, err, "failed to update policy %+v", fwPolicyEvent.Policy)
		case api.EventType_DeleteEvent:
			err := ps.DeleteFwLogPolicy(ctx, fwPolicyEvent.Policy)
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
					facility:    syslog.Priority(monitoring.SyslogFacility_value[policySpec.Config.FacilityOverride]),
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
	}
}

func TestSyslogConnect(t *testing.T) {
	ps := &PolicyState{
		fwLogCollectors: sync.Map{},
	}

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
	ctx, cancel := context.WithCancel(context.Background())
	err = ps.connectSyslog(ctx)
	AssertOk(t, err, "failed to start recconnect-syslog")
	time.Sleep(time.Second)

	AssertEventually(t, func() (bool, interface{}) {
		server := map[string]bool{}
		ps.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {
			if val, ok := v.(*fwlogCollector); ok {
				if val.syslogFd == nil {
					server[k.(string)] = true
				}
			}
			return true
		})
		return len(server) == 0, server

	}, "syslog reconnect failed")

	cancel()
	ps.wg.Wait()
}

func TestUnusedCb(t *testing.T) {
	l, err := startNetagent()
	AssertOk(t, err, "failed to create mock netagent")
	defer l.Close()

	/// init tsdb
	opt := &tsdb.Opts{
		ClientName:     "netagent-007",
		ResolverClient: &mock.ResolverClient{},
	}
	ctx, cancel := context.WithCancel(context.Background())

	tsdbCtx, tsdbCancel := context.WithCancel(ctx)
	tsdb.Init(tsdbCtx, opt)
	tsdbCancel() //abort tsdb transmit

	ps, err := NewTpAgent(ctx, strings.Split(l.Addr().(*net.TCPAddr).String(), ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")
	defer ps.Close()
	defer cancel()

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
