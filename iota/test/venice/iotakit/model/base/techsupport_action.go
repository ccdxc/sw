// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package base

import (
	"context"
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
)

// VerifyTechsupportStatus verifies if techsupport got accepted
func (sm *SysModel) VerifyTechsupportStatus(techsupportName string) error {
	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 10*time.Minute)
	defer cancelFunc()
	ctx, err := sm.VeniceLoggedInCtx(bkCtx)

	if err != nil {
		return err
	}

	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	obj := api.ObjectMeta{Name: techsupportName, Tenant: "default"}
	resp, err := restcls[0].MonitoringV1().TechSupportRequest().Get(ctx, &obj)
	if err != nil || resp.Name != techsupportName {
		return fmt.Errorf("ts:%s Techsupport [%s] read failed, err: %+v response: %+v\n", time.Now().String(), techsupportName, err, resp)
	}
	return nil
}

// VerifyTechsupport verifies if techsupport got completed
func (sm *SysModel) VerifyTechsupport(techsupportName string) error {
	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 10*time.Minute)
	defer cancelFunc()
	ctx, err := sm.VeniceLoggedInCtx(bkCtx)

	if err != nil {
		return err
	}

	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	obj := api.ObjectMeta{Name: techsupportName, Tenant: "default"}
	resp, err := restcls[0].MonitoringV1().TechSupportRequest().Get(ctx, &obj)
	if err != nil || resp.Name != techsupportName {
		return fmt.Errorf("ts:%s Techsupport [%s] read failed, err: %+v response: %+v\n", time.Now().String(), techsupportName, err, resp)
	}
	if resp.Status.Status != "completed" {
		return fmt.Errorf("ts:%s Techsupport [%s] status mismatch, resp: %+v\n", time.Now().String(), techsupportName, resp)
	}
	return nil
}

// PerformTechsupport performs a techsupport in the cluster
func (sm *SysModel) PerformTechsupport(techsupport *monitoring.TechSupportRequest) error {
	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 15*time.Minute)
	defer cancelFunc()
	ctx, err := sm.VeniceLoggedInCtx(bkCtx)
	if err != nil {
		return err
	}

	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	//create the techsupport request object
	r1, err := restcls[0].MonitoringV1().TechSupportRequest().Create(ctx, techsupport)
	if err != nil || r1.Name != techsupport.Name {
		return err
	}

	return nil
}

// DeleteTechsupport deletes the techsupport
func (sm *SysModel) DeleteTechsupport(techsupportName string) error {
	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 1*time.Minute)
	defer cancelFunc()
	ctx, err := sm.VeniceLoggedInCtx(bkCtx)
	if err != nil {
		return err
	}

	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	tsObj := api.ObjectMeta{Name: techsupportName, Tenant: "default"}
	// delete the techsupport request object
	resp, err := restcls[0].MonitoringV1().TechSupportRequest().Delete(ctx, &tsObj)
	if err != nil || resp.Name != techsupportName {
		return fmt.Errorf("ts:%s Techsupport [%s] deletion failed, err: %+v response %v", time.Now().String(), techsupportName, err, resp)
	}

	return nil
}
