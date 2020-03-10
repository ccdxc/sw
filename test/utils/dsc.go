package utils

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
)

// SetDSCProfile sets the profile on a DSC object
func (tu *TestUtils) SetDSCProfile(ctx context.Context, dscObjMeta *api.ObjectMeta, profURI string) error {
	client, err := apiclient.NewRestAPIClient(tu.APIGwAddr)
	if err != nil {
		return fmt.Errorf("Error creating new API Client, addr: %s, err: %v", tu.APIGwAddr, err)
	}
	dsc, err := client.ClusterV1().DistributedServiceCard().Get(ctx, dscObjMeta)
	if err != nil {
		return fmt.Errorf("Error getting DSC object: %v, err: %v", dscObjMeta, err)
	}
	dsc.Spec.DSCProfile = profURI
	_, err = client.ClusterV1().DistributedServiceCard().Update(ctx, dsc)
	if err != nil {
		return fmt.Errorf("Error updating DSC object: %v, err: %v", dscObjMeta, err)
	}
	return nil
}
