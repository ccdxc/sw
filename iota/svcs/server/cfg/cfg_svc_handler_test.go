package cfg

import (
	"context"
	"fmt"
	"os"
	"testing"
	"time"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/utils/netutils"
)

var cfgClient iota.ConfigMgmtApiClient
var cfgServerURL string

func startCfgService() {
	var testListener netutils.TestListenAddr
	err := testListener.GetAvailablePort()
	if err != nil {
		fmt.Println("Could not get an available port")
		os.Exit(1)
	}

	cfgServerURL = testListener.ListenURL.String()

	cfgSvc, err := common.CreateNewGRPCServer("IOTA Server", cfgServerURL)
	if err != nil {
		fmt.Printf("Could not start Config Service. Err: %v\n", err)
		os.Exit(1)
	}

	cfgHandler := NewConfigServiceHandler()

	iota.RegisterConfigMgmtApiServer(cfgSvc.Srv, cfgHandler)

	cfgSvc.Start()
}

func TestMain(m *testing.M) {
	go func() {
		startCfgService()
	}()

	// Ensure the service is up
	time.Sleep(time.Second * 2)

	c, err := common.CreateNewGRPCClient("cfg-test-client", cfgServerURL)
	if err != nil {
		fmt.Println("Could not create a GRPC Client to the IOTA Server")
		os.Exit(1)
	}

	cfgClient = iota.NewConfigMgmtApiClient(c.Client)

	runTests := m.Run()
	os.Exit(runTests)

}

func TestConfigService_InitCfgService(t *testing.T) {
	t.Parallel()
	var cfgMsg iota.InitConfigMsg

	_, err := cfgClient.InitCfgService(context.Background(), &cfgMsg)
	if err != nil {
		t.Errorf("InitCfgService call failed. Err: %v", err)
	}
}

func TestConfigService_GenerateConfigs(t *testing.T) {
	t.Parallel()
	var cfgMsg iota.GenerateConfigMsg

	_, err := cfgClient.GenerateConfigs(context.Background(), &cfgMsg)
	if err != nil {
		t.Errorf("GenerateConfigs call failed. Err: %v", err)
	}
}

func TestConfigService_PushConfig(t *testing.T) {
	t.Parallel()
	var cfgMsg iota.ConfigMsg

	_, err := cfgClient.PushConfig(context.Background(), &cfgMsg)
	if err != nil {
		t.Errorf("PushConfig call failed. Err: %v", err)
	}
}

func TestConfigService_QueryConfig(t *testing.T) {
	t.Parallel()
	var cfgMsg iota.ConfigQueryMsg

	_, err := cfgClient.QueryConfig(context.Background(), &cfgMsg)
	if err != nil {
		t.Errorf("PushConfig call failed. Err: %v", err)
	}
}
