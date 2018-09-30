package server

import (
	"context"
	"testing"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

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
