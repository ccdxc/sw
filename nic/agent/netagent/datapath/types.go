package datapath

import (
	"sync"

	"github.com/golang/mock/gomock"
	"google.golang.org/grpc"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
)

// Kind holds the HAL Datapath kind. It could either be mock HAL or real HAL.
type Kind string

// Hal holds clients to talk to HAL gRPC server. ToDo Remove mock clients, datapath DB and expectations prior to FCS
type Hal struct {
	client               *grpc.ClientConn
	mockCtrl             *gomock.Controller
	StateAPI             types.CtrlerIntf
	MockClients          mockClients
	Epclient             halproto.EndpointClient
	Ifclient             halproto.InterfaceClient
	L2SegClient          halproto.L2SegmentClient
	Netclient            halproto.NetworkClient
	Lbclient             halproto.L4LbClient
	Sgclient             halproto.NwSecurityClient
	Sessclient           halproto.SessionClient
	Tnclient             halproto.VrfClient
	Natclient            halproto.NatClient
	IPSecclient          halproto.IpsecClient
	PortClient           halproto.PortClient
	TCPProxyPolicyClient halproto.TcpProxyClient
	SystemClient         halproto.SystemClient
	EventClient          halproto.EventClient
}

// MockClients stores references for mockclients to be used for setting expectations
type mockClients struct {
	MockEpclient       *halproto.MockEndpointClient
	MockIfclient       *halproto.MockInterfaceClient
	MockL2Segclient    *halproto.MockL2SegmentClient
	MockNetClient      *halproto.MockNetworkClient
	MockLbclient       *halproto.MockL4LbClient
	MockSgclient       *halproto.MockNwSecurityClient
	MockSessclient     *halproto.MockSessionClient
	MockTnclient       *halproto.MockVrfClient
	MockNatClient      *halproto.MockNatClient
	MockIPSecClient    *halproto.MockIpsecClient
	MockTCPProxyClient *halproto.MockTcpProxyClient
	MockPortClient     *halproto.MockPortClient
	MockSystemClient   *halproto.MockSystemClient
	MockEventClient    *halproto.MockEventClient
}

// DB holds all the state information.
type DB struct {
	EndpointDB       map[string]*halproto.EndpointRequestMsg
	EndpointUpdateDB map[string]*halproto.EndpointUpdateRequestMsg
	EndpointDelDB    map[string]*halproto.EndpointDeleteRequestMsg
	SgDB             map[string]*halproto.SecurityGroupRequestMsg
	TenantDB         map[string]*halproto.VrfRequestMsg
	TenantDelDB      map[string]*halproto.VrfDeleteRequestMsg
	SgPolicyDB       map[string]*halproto.SecurityPolicyRequestMsg
	InterfaceDB      map[string]*halproto.InterfaceRequestMsg
	LifDB            map[string]*halproto.LifRequestMsg
	InterfaceDelDB   map[string]*halproto.InterfaceDeleteRequestMsg
	LifDelDB         map[string]*halproto.LifDeleteRequestMsg
}

// Datapath contains mock and hal clients.
type Datapath struct {
	sync.Mutex
	Kind Kind
	Hal  Hal
	DB   DB
}
