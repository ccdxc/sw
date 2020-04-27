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

// SecurityRuleGet stubbed out
func (h Hal) SecurityRuleGet(ctx context.Context, req *apuluproto.SecurityRuleGetRequest) (*apuluproto.SecurityRuleGetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityRuleGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityProfileGet stubbed out
func (h Hal) SecurityProfileGet(ctx context.Context, req *apuluproto.SecurityProfileGetRequest) (*apuluproto.SecurityProfileGetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityProfileGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityPolicyGet stubbed out
func (h Hal) SecurityPolicyGet(ctx context.Context, req *apuluproto.SecurityPolicyGetRequest) (*apuluproto.SecurityPolicyGetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityPolicyGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SubnetGet stubbed out
func (h Hal) SubnetGet(ctx context.Context, req *apuluproto.SubnetGetRequest) (*apuluproto.SubnetGetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SubnetGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// MirrorSessionGet stubbed out
func (h Hal) MirrorSessionGet(ctx context.Context, req *apuluproto.MirrorSessionGetRequest) (*apuluproto.MirrorSessionGetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got MirrorSessionGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

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

// DHCPPolicyGet stubbed out
func (h Hal) DHCPPolicyGet(ctx context.Context, req *apuluproto.DHCPPolicyGetRequest) (*apuluproto.DHCPPolicyGetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got DHCPPolicyGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}
