// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package datapath

import (
	"os"
	"time"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	halGRPCDefaultBaseURL = "localhost"
	halGRPCDefaultPort    = "50054"
	halGRPCWaitTimeout    = time.Duration(time.Minute * 10)
)

// String returns string value of the datapath kind
func (k *Kind) String() string {
	return string(*k)
}

// Errorf for satisfying gomock
func (hd *Hal) Errorf(format string, args ...interface{}) {
	log.Errorf(format, args...)
}

// Fatalf for satisfying gomock
func (hd *Hal) Fatalf(format string, args ...interface{}) {
	log.Fatalf(format, args...)
}

// NewHalDatapath returns a either a mock or a real hal datapath
func NewHalDatapath(kind Kind) (*Datapath, error) {
	var err error
	var hal Hal
	haldp := Datapath{}
	haldp.Kind = kind

	db := DB{EndpointDB: make(map[string]*halproto.EndpointRequestMsg),
		EndpointUpdateDB: make(map[string]*halproto.EndpointUpdateRequestMsg),
		EndpointDelDB:    make(map[string]*halproto.EndpointDeleteRequestMsg),
		SgDB:             make(map[string]*halproto.SecurityGroupRequestMsg),
		TenantDB:         make(map[string]*halproto.VrfRequestMsg),
		TenantDelDB:      make(map[string]*halproto.VrfDeleteRequestMsg),
		SgPolicyDB:       make(map[string]*halproto.SecurityGroupPolicyRequestMsg),
		InterfaceDB:      make(map[string]*halproto.InterfaceRequestMsg),
		LifDB:            make(map[string]*halproto.LifRequestMsg),
		InterfaceDelDB:   make(map[string]*halproto.InterfaceDeleteRequestMsg),
		LifDelDB:         make(map[string]*halproto.LifDeleteRequestMsg),
	}
	haldp.DB = db
	if haldp.Kind.String() == "hal" {
		hal.client, err = hal.createNewGRPCClient()
		if err != nil {
			return nil, err
		}
		hal.Epclient = halproto.NewEndpointClient(hal.client.ClientConn)
		hal.Ifclient = halproto.NewInterfaceClient(hal.client.ClientConn)
		hal.L2SegClient = halproto.NewL2SegmentClient(hal.client.ClientConn)
		hal.Netclient = halproto.NewNetworkClient(hal.client.ClientConn)
		hal.Lbclient = halproto.NewL4LbClient(hal.client.ClientConn)
		hal.Sgclient = halproto.NewNwSecurityClient(hal.client.ClientConn)
		hal.Sessclient = halproto.NewSessionClient(hal.client.ClientConn)
		hal.Tnclient = halproto.NewVrfClient(hal.client.ClientConn)
		hal.Natclient = halproto.NewNatClient(hal.client.ClientConn)
		hal.IPSecclient = halproto.NewIpsecClient(hal.client.ClientConn)
		haldp.Hal = hal
		return &haldp, nil
	}
	hal.mockCtrl = gomock.NewController(&hal)
	hal.MockClients = mockClients{
		MockEpclient:    halproto.NewMockEndpointClient(hal.mockCtrl),
		MockIfclient:    halproto.NewMockInterfaceClient(hal.mockCtrl),
		MockL2Segclient: halproto.NewMockL2SegmentClient(hal.mockCtrl),
		MockNetClient:   halproto.NewMockNetworkClient(hal.mockCtrl),
		MockLbclient:    halproto.NewMockL4LbClient(hal.mockCtrl),
		MockSgclient:    halproto.NewMockNwSecurityClient(hal.mockCtrl),
		MockSessclient:  halproto.NewMockSessionClient(hal.mockCtrl),
		MockTnclient:    halproto.NewMockVrfClient(hal.mockCtrl),
		MockNatClient:   halproto.NewMockNatClient(hal.mockCtrl),
		MockIPSecClient: halproto.NewMockIpsecClient(hal.mockCtrl),
	}

	hal.Epclient = hal.MockClients.MockEpclient
	hal.Ifclient = hal.MockClients.MockIfclient
	hal.L2SegClient = hal.MockClients.MockL2Segclient
	hal.Netclient = hal.MockClients.MockNetClient
	hal.Lbclient = hal.MockClients.MockLbclient
	hal.Sgclient = hal.MockClients.MockSgclient
	hal.Sessclient = hal.MockClients.MockSessclient
	hal.Tnclient = hal.MockClients.MockTnclient
	hal.Natclient = hal.MockClients.MockNatClient
	hal.IPSecclient = hal.MockClients.MockIPSecClient
	haldp.Hal = hal
	haldp.Hal.setExpectations()
	return &haldp, nil
}

func (hd *Hal) setExpectations() {
	hd.MockClients.MockEpclient.EXPECT().EndpointCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockEpclient.EXPECT().EndpointUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockEpclient.EXPECT().EndpointDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockIfclient.EXPECT().InterfaceGet(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().AddL2SegmentOnUplink(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().InterfaceCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().InterfaceUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().InterfaceDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockIfclient.EXPECT().LifGet(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().LifCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().LifUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().LifDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockL2Segclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockL2Segclient.EXPECT().L2SegmentUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockL2Segclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNetClient.EXPECT().NetworkCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().NetworkUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().NetworkDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNetClient.EXPECT().RouteCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().RouteUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().RouteDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNetClient.EXPECT().NexthopCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().NexthopUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().NexthopDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockSgclient.EXPECT().SecurityGroupCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockSgclient.EXPECT().SecurityGroupUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockSgclient.EXPECT().SecurityGroupDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockTnclient.EXPECT().VrfUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockTnclient.EXPECT().VrfDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNatClient.EXPECT().NatPoolCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNatClient.EXPECT().NatPoolUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNatClient.EXPECT().NatPoolDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNatClient.EXPECT().NatPolicyCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNatClient.EXPECT().NatPolicyUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNatClient.EXPECT().NatPolicyDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNatClient.EXPECT().NatMappingCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNatClient.EXPECT().NatMappingDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockIPSecClient.EXPECT().IpsecRuleCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecRuleUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecRuleDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockIPSecClient.EXPECT().IpsecSAEncryptCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecSAEncryptUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecSAEncryptDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockIPSecClient.EXPECT().IpsecSADecryptCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecSADecryptUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecSADecryptDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
}

func (hd *Hal) createNewGRPCClient() (*rpckit.RPCClient, error) {
	halPort := os.Getenv("HAL_GRPC_PORT")
	if halPort == "" {
		halPort = halGRPCDefaultPort
	}
	srvURL := halGRPCDefaultBaseURL + ":" + halPort
	// create a grpc client
	// ToDo Use AgentID for mysvcName
	return hd.waitForHAL(srvURL)
}

func (hd *Hal) waitForHAL(halURL string) (rpcClient *rpckit.RPCClient, err error) {
	halUP := make(chan bool, 1)
	ticker := time.NewTicker(time.Second * 10)
	timeout := time.After(halGRPCWaitTimeout)

	for {
		select {
		case <-ticker.C:
			log.Debugf("Trying to connect to HAL at %s", halURL)
			rpcClient, err = rpckit.NewRPCClient("hal", halURL, rpckit.WithTLSProvider(nil))
			if err == nil {
				halUP <- true
			}
		case <-halUP:
			log.Info("Agent is connected to HAL")
			return
		case <-timeout:
			log.Errorf("Agent could not connect to HAL on %s. Err: %v", halURL, err)
			return nil, ErrHALUnavailable
		}
	}
}

// SetAgent sets the agent for this datapath
func (hd *Datapath) SetAgent(ag types.DatapathIntf) error {
	return nil
}
