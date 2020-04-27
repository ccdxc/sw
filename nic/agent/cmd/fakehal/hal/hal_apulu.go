// +build apulu

package fakehal

import (
	"context"
	"encoding/json"
	"fmt"
	"net"
	"time"

	"google.golang.org/grpc"

	apuluproto "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// Hal implements fake hal svc layer
type Hal struct {
	ListenURL string
	Srv       *grpc.Server
}

// ########################## HAL methods used by DSCAgent

// Stop stops the Fake HAL
func (h Hal) Stop() {
	log.Infof("Stopping HAL at: %s", h.ListenURL)
	if h.Srv != nil {
		h.Srv.Stop()
	}
}

// InterfaceCreate stubbed out
func (h Hal) InterfaceCreate(ctx context.Context, req *apuluproto.InterfaceRequest) (*apuluproto.InterfaceResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceCreate Request:")
	fmt.Println(string(dat))

	return &apuluproto.InterfaceResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// InterfaceUpdate stubbed out
func (h Hal) InterfaceUpdate(ctx context.Context, req *apuluproto.InterfaceRequest) (*apuluproto.InterfaceResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceUpdate Request:")
	fmt.Println(string(dat))

	return &apuluproto.InterfaceResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// InterfaceDelete stubbed out
func (h Hal) InterfaceDelete(ctx context.Context, req *apuluproto.InterfaceDeleteRequest) (*apuluproto.InterfaceDeleteResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceDelete Request:")
	fmt.Println(string(dat))

	return &apuluproto.InterfaceDeleteResponse{
		ApiStatus: []apuluproto.ApiStatus{
			apuluproto.ApiStatus_API_STATUS_OK,
		},
	}, nil
}

// SecurityRuleCreate stubbed out
func (h Hal) SecurityRuleCreate(ctx context.Context, req *apuluproto.SecurityRuleRequest) (*apuluproto.SecurityRuleResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityRuleCreate Request:")
	fmt.Println(string(dat))

	return &apuluproto.SecurityRuleResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SecurityRuleUpdate stubbed out
func (h Hal) SecurityRuleUpdate(ctx context.Context, req *apuluproto.SecurityRuleRequest) (*apuluproto.SecurityRuleResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityRuleUpdate Request:")
	fmt.Println(string(dat))

	return &apuluproto.SecurityRuleResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SecurityRuleDelete stubbed out
func (h Hal) SecurityRuleDelete(ctx context.Context, req *apuluproto.SecurityRuleDeleteRequest) (*apuluproto.SecurityRuleDeleteResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityRuleDelete Request:")
	fmt.Println(string(dat))

	return &apuluproto.SecurityRuleDeleteResponse{
		ApiStatus: []apuluproto.ApiStatus{
			apuluproto.ApiStatus_API_STATUS_OK,
		},
	}, nil
}

// SecurityProfileCreate stubbed out
func (h Hal) SecurityProfileCreate(ctx context.Context, req *apuluproto.SecurityProfileRequest) (*apuluproto.SecurityProfileResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityProfileCreate Request:")
	fmt.Println(string(dat))

	return &apuluproto.SecurityProfileResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SecurityProfileUpdate stubbed out
func (h Hal) SecurityProfileUpdate(ctx context.Context, req *apuluproto.SecurityProfileRequest) (*apuluproto.SecurityProfileResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityProfileUpdate Request:")
	fmt.Println(string(dat))

	return &apuluproto.SecurityProfileResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SecurityProfileDelete stubbed out
func (h Hal) SecurityProfileDelete(ctx context.Context, req *apuluproto.SecurityProfileDeleteRequest) (*apuluproto.SecurityProfileDeleteResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityProfileDelete Request:")
	fmt.Println(string(dat))

	return &apuluproto.SecurityProfileDeleteResponse{
		ApiStatus: []apuluproto.ApiStatus{
			apuluproto.ApiStatus_API_STATUS_OK,
		},
	}, nil
}

// SecurityPolicyCreate stubbed out
func (h Hal) SecurityPolicyCreate(ctx context.Context, req *apuluproto.SecurityPolicyRequest) (*apuluproto.SecurityPolicyResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityPolicyCreate Request:")
	fmt.Println(string(dat))

	return &apuluproto.SecurityPolicyResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SecurityPolicyUpdate stubbed out
func (h Hal) SecurityPolicyUpdate(ctx context.Context, req *apuluproto.SecurityPolicyRequest) (*apuluproto.SecurityPolicyResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityPolicyUpdate Request:")
	fmt.Println(string(dat))

	return &apuluproto.SecurityPolicyResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SecurityPolicyDelete stubbed out
func (h Hal) SecurityPolicyDelete(ctx context.Context, req *apuluproto.SecurityPolicyDeleteRequest) (*apuluproto.SecurityPolicyDeleteResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityPolicyDelete Request:")
	fmt.Println(string(dat))

	return &apuluproto.SecurityPolicyDeleteResponse{
		ApiStatus: []apuluproto.ApiStatus{
			apuluproto.ApiStatus_API_STATUS_OK,
		},
	}, nil
}

// SubnetCreate stubbed out
func (h Hal) SubnetCreate(ctx context.Context, req *apuluproto.SubnetRequest) (*apuluproto.SubnetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SubnetCreate Request:")
	fmt.Println(string(dat))

	return &apuluproto.SubnetResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SubnetUpdate stubbed out
func (h Hal) SubnetUpdate(ctx context.Context, req *apuluproto.SubnetRequest) (*apuluproto.SubnetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SubnetUpdate Request:")
	fmt.Println(string(dat))

	return &apuluproto.SubnetResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SubnetDelete stubbed out
func (h Hal) SubnetDelete(ctx context.Context, req *apuluproto.SubnetDeleteRequest) (*apuluproto.SubnetDeleteResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SubnetDelete Request:")
	fmt.Println(string(dat))

	return &apuluproto.SubnetDeleteResponse{
		ApiStatus: []apuluproto.ApiStatus{
			apuluproto.ApiStatus_API_STATUS_OK,
		},
	}, nil
}

// MirrorSessionCreate stubbed out
func (h Hal) MirrorSessionCreate(ctx context.Context, req *apuluproto.MirrorSessionRequest) (*apuluproto.MirrorSessionResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got MirrorSessionCreate Request:")
	fmt.Println(string(dat))

	return &apuluproto.MirrorSessionResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// MirrorSessionUpdate stubbed out
func (h Hal) MirrorSessionUpdate(ctx context.Context, req *apuluproto.MirrorSessionRequest) (*apuluproto.MirrorSessionResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got MirrorSessionUpdate Request:")
	fmt.Println(string(dat))

	return &apuluproto.MirrorSessionResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// MirrorSessionDelete stubbed out
func (h Hal) MirrorSessionDelete(ctx context.Context, req *apuluproto.MirrorSessionDeleteRequest) (*apuluproto.MirrorSessionDeleteResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got MirrorSessionDelete Request:")
	fmt.Println(string(dat))

	return &apuluproto.MirrorSessionDeleteResponse{
		ApiStatus: []apuluproto.ApiStatus{
			apuluproto.ApiStatus_API_STATUS_OK,
		},
	}, nil
}

// DHCPPolicyCreate stubbed out
func (h Hal) DHCPPolicyCreate(ctx context.Context, req *apuluproto.DHCPPolicyRequest) (*apuluproto.DHCPPolicyResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got DHCPPolicyCreate Request:")
	fmt.Println(string(dat))

	return &apuluproto.DHCPPolicyResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// DHCPPolicyUpdate stubbed out
func (h Hal) DHCPPolicyUpdate(ctx context.Context, req *apuluproto.DHCPPolicyRequest) (*apuluproto.DHCPPolicyResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got DHCPPolicyUpdate Request:")
	fmt.Println(string(dat))

	return &apuluproto.DHCPPolicyResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// DHCPPolicyDelete stubbed out
func (h Hal) DHCPPolicyDelete(ctx context.Context, req *apuluproto.DHCPPolicyDeleteRequest) (*apuluproto.DHCPPolicyDeleteResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got DHCPPolicyDelete Request:")
	fmt.Println(string(dat))

	return &apuluproto.DHCPPolicyDeleteResponse{
		ApiStatus: []apuluproto.ApiStatus{
			apuluproto.ApiStatus_API_STATUS_OK,
		},
	}, nil
}

// NewFakeHalServer returns a fake hal server on a specified port
func NewFakeHalServer(listenURL string) *Hal {
	lis, err := net.Listen("tcp", listenURL)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	f := Hal{
		ListenURL: listenURL,
		Srv:       grpc.NewServer(),
	}

	apuluproto.RegisterIfSvcServer(f.Srv, &f)
	apuluproto.RegisterMirrorSvcServer(f.Srv, &f)
	apuluproto.RegisterSubnetSvcServer(f.Srv, &f)
	apuluproto.RegisterSecurityPolicySvcServer(f.Srv, &f)
	apuluproto.RegisterDHCPSvcServer(f.Srv, &f)
	go f.Srv.Serve(lis)

	time.Sleep(time.Second * 2)
	log.Infof("Fake HAL Started at: %s", f.ListenURL)
	return &f
}
