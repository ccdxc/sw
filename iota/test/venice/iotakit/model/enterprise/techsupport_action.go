// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package enterprise

import (
	"context"
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"

	"github.com/pensando/sw/venice/utils/log"
)

// VerifyTechsupportStatus verifies status of techsupport in the iota cluster
func (sm *SysModel) VerifyTechsupportStatus(techsupportName string) error {
	var numRetries int
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

	for numRetries = 0; numRetries < 24; numRetries++ {
		obj := api.ObjectMeta{Name: techsupportName, Tenant: "default"}
		r1, err := restcls[0].MonitoringV1().TechSupportRequest().Get(ctx, &obj)
		if err != nil || r1.Name != techsupportName {
			log.Errorf("ts:%s Techsupport failed for [%s] err: %+v r1: %+v", time.Now().String(), techsupportName, err, r1)
			time.Sleep(time.Second)
			continue
		}
		log.Infof("ts:%s Techsupport GET validated for [%s]", time.Now().String(), techsupportName)
		numRetries = 0
		break
	}

	if numRetries != 0 {
		return fmt.Errorf("Failed to create techsupport object")
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
