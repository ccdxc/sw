package validator

import (
	"io/ioutil"
	"os"
	"testing"

	"github.com/pensando/sw/nic/agent/dscagent/infra"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/venice/utils/log"
)

var infraAPI types.InfraAPI

func TestMain(m *testing.M) {
	primaryDB, err := ioutil.TempFile("", "")
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	secondaryDB, err := ioutil.TempFile("", "")
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	infraAPI, err = infra.NewInfraAPI(primaryDB.Name(), secondaryDB.Name())
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}
	code := m.Run()
	infraAPI.Close()
	os.Remove(primaryDB.Name())
	os.Remove(secondaryDB.Name())
	os.Exit(code)
}
