// +build apulu

package fakehal

import (
	"context"
	"encoding/json"
	"fmt"

	apuluproto "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// ##########################n HAL methods stubed out to satisfy interface

// InterfaceGet stubbed out
func (h Hal) InterfaceGet(ctx context.Context, req *apuluproto.InterfaceGetRequest) (*apuluproto.InterfaceGetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// LifGet stubbed out
func (h Hal) LifGet(ctx context.Context, req *apuluproto.LifGetRequest) (*apuluproto.LifGetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got LifGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}
