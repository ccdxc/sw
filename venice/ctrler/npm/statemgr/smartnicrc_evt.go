// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/utils/featureflags"
	"github.com/pensando/sw/venice/utils/log"
)

var dscMgrRc *DSCMgrRc

// DSCMgrRc is object manager for distriibuted service card with routing config
type DSCMgrRc struct {
	featureMgrBase
	sm *Statemgr
}

// CompleteRegistration is the callback function statemgr calls after init is done
func (dscMgr *DSCMgrRc) CompleteRegistration() {
	if featureflags.IsOVerlayRoutingEnabled() == false {
		return
	}

	dscMgr.sm.SetDistributedServiceCardReactor(dscMgr)
}

func init() {
	mgr := MustGetStatemgr()
	dscMgrRc = &DSCMgrRc{
		sm: mgr,
	}

	mgr.Register("statemgrsmartnicrc", dscMgrRc)
}

//GetDistributedServiceCardWatchOptions gets options
func (dscMgr *DSCMgrRc) GetDistributedServiceCardWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{}
	return &opts
}

// OnDistributedServiceCardCreate handles smartNic creation
func (dscMgr *DSCMgrRc) OnDistributedServiceCardCreate(smartNic *ctkit.DistributedServiceCard) error {
	defer dscMgr.sm.sendDscUpdateNotification(&smartNic.DistributedServiceCard)
	defer dscMgr.sm.ProcessDSCEvent(CreateEvent, &smartNic.DistributedServiceCard)
	sns, err := dscMgr.sm.dscCreate(smartNic)
	if err != nil {
		return err
	}
	dscMgr.sm.addDSCRelatedobjects(smartNic, sns, false)
	dscMgr.sm.PeriodicUpdaterPush(sns)
	if smartNic.Spec.RoutingConfig != "" {
		log.Infof("Sending routing config dsc: %s | rtcfg: %s", smartNic.Name, smartNic.Spec.RoutingConfig)
		dscMgr.sm.mbus.SendRoutingConfig(smartNic.Name, "", smartNic.Spec.RoutingConfig)
	}
	return nil
}

// OnDistributedServiceCardUpdate handles update event on smartnic
func (dscMgr *DSCMgrRc) OnDistributedServiceCardUpdate(smartNic *ctkit.DistributedServiceCard, nsnic *cluster.DistributedServiceCard) error {
	defer dscMgr.sm.sendDscUpdateNotification(nsnic)
	defer dscMgr.sm.ProcessDSCEvent(UpdateEvent, &smartNic.DistributedServiceCard)
	log.Infof("DSC spec old: %+v | new: %+v", smartNic.Spec, nsnic.Spec)
	oldRt := smartNic.Spec.RoutingConfig
	newRt := nsnic.Spec.RoutingConfig

	sns, err := dscMgr.sm.updateDSC(smartNic, nsnic)
	if err != nil {
		log.Errorf("updateDsc returned error: %s", err)
		return nil
	}

	dscMgr.sm.updateDSCRelatedObjects(sns, nsnic, false)
	dscMgr.sm.PeriodicUpdaterPush(sns)
	if oldRt != newRt {
		log.Infof("Sending routing config dsc: %s | rtcfg old: %s, new: %s", smartNic.Name, oldRt, newRt)
		dscMgr.sm.mbus.SendRoutingConfig(smartNic.Name, oldRt, newRt)
	}
	return nil
}

// OnDistributedServiceCardDelete handles smartNic deletion
func (dscMgr *DSCMgrRc) OnDistributedServiceCardDelete(smartNic *ctkit.DistributedServiceCard) error {
	defer dscMgr.sm.ProcessDSCEvent(DeleteEvent, &smartNic.DistributedServiceCard)
	hs, err := dscMgr.sm.deleteDsc(smartNic)
	if err != nil {
		return err
	}
	log.Infof("Sending routing config dsc: %s | rtcfg: %s", smartNic.Name, smartNic.Spec.RoutingConfig)
	dscMgr.sm.mbus.SendRoutingConfig(smartNic.Name, smartNic.Spec.RoutingConfig, "")
	return dscMgr.sm.deleteDscRelatedObjects(smartNic, hs, false)

}

// OnDistributedServiceCardReconnect is called when ctkit reconnects to apiserver
func (dscMgr *DSCMgrRc) OnDistributedServiceCardReconnect() {
	return
}
