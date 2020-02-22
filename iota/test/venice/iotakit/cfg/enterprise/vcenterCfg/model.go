package vcentercfg

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"time"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/base"
	"github.com/pensando/sw/venice/utils/log"
)

const configFile = "/tmp/scale-cfg.json"

//VcenterCfg encapsulate all Vcenter configuration objects
type VcenterCfg struct {
	base.EntBaseCfg
}

//NewVcenterCfg new vcenter config
func NewVcenterCfg() *VcenterCfg {
	return &VcenterCfg{}
}

func (vc *VcenterCfg) pushConfigViaRest() error {

	cfg := vc.Cfg

	rClient := vc.Client

	/*
		createHosts := func() error {
			defer base.TimeTrack(time.Now(), "Creating hosts")
			for _, o := range cfg.ConfigItems.Hosts {
				err := rClient.CreateHost(o)
				if err != nil {
					log.Errorf("Error creating host: %+v. Err: %v", o, err)
					return err
				}
			}
			return nil
		}

		if err := createHosts(); err != nil {
			return err
		} */

	//Set mac address to be empty as venter and orch will figure out for themselves
	for _, wl := range vc.Cfg.ConfigItems.Workloads {
		for index := range wl.Spec.Interfaces {
			wl.Spec.Interfaces[index].MACAddress = ""
		}
	}

	for _, o := range cfg.ConfigItems.Networks {

		if err := rClient.CreateNetwork(o); err != nil {
			log.Errorf("Error creating network %s", err)
			//Ignore network create error as it might be created already
			//	return fmt.Errorf("error creating network: %s", err)
		}
	}

	for _, o := range cfg.ConfigItems.Apps {
		if err := rClient.CreateApp(o); err != nil {
			return fmt.Errorf("error creating app: %s", err)
		}
	}

	for _, o := range cfg.ConfigItems.SGPolicies {
		createSgPolicy := func() error {
			defer base.TimeTrack(time.Now(), "Create Sg Policy")
			if err := rClient.CreateNetworkSecurityPolicy(o); err != nil {
				return fmt.Errorf("error creating sgpolicy: %s", err)
			}
			return nil
		}
		if err := createSgPolicy(); err != nil {
			return err
		}
	}

	if len(cfg.ConfigItems.SGPolicies) > 1 {
		panic("can't have more than one sgpolicy")
	}

	for _, o := range cfg.ConfigItems.SGPolicies {
		createSgPolicy := func() error {
			defer base.TimeTrack(time.Now(), "Create Sg Policy")
			if err := rClient.CreateNetworkSecurityPolicy(o); err != nil {
				return fmt.Errorf("error creating sgpolicy: %s", err)
			}
			return nil
		}
		if err := createSgPolicy(); err != nil {
			return err
		}
	}

	//Append default Sg polcies
	vc.DefaultSgPolicies = []*base.NetworkSecurityPolicy{}
	for _, sgPolicy := range vc.Cfg.ConfigItems.SGPolicies {
		log.Infof("Setting up default sg policicies........... \n")
		vc.DefaultSgPolicies = append(vc.DefaultSgPolicies, &base.NetworkSecurityPolicy{VenicePolicy: sgPolicy})
	}

	return nil
}

//CleanupAllConfig cleans up config which is independent
func (vc *VcenterCfg) CleanupAllConfig() error {
	var err error

	rClient := vc.Client

	// get all venice configs
	veniceHosts, err := rClient.ListHost()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

	veniceSGPolicies, err := rClient.ListNetworkSecurityPolicy()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceNetworks, err := rClient.ListNetwork("")
	if err != nil {
		//Deleting network might fail as some workloads may be hanging of it
		log.Errorf("err: %s", err)
		//return err
	}
	veniceApps, err := rClient.ListApp()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceWorkloads, err := rClient.ListWorkload()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

	log.Infof("Cleanup: Apps %d, sgpolicy %d networks %d",
		len(veniceApps), len(veniceSGPolicies), len(veniceNetworks))

	// delete venice objects
	for _, obj := range veniceSGPolicies {
		if err := rClient.DeleteNetworkSecurityPolicy(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}
	for _, obj := range veniceApps {
		if err := rClient.DeleteApp(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}

	if err := rClient.DeleteWorkloads(veniceWorkloads); err != nil {
		err = fmt.Errorf("Error deleting workloads Err: %v", err)
		log.Errorf("%s", err)
		return err
	}

	for i := 0; i < 1; i++ {

	}

	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 10*time.Minute)
	defer cancelFunc()
L:
	for true {
		select {
		case <-bkCtx.Done():
			return fmt.Errorf("Error deleting all endpoints: %s", err)
		default:
			veniceEndpoints, err := rClient.ListEndpoints(globals.DefaultTenant)
			if err != nil {
				log.Errorf("err: %s", err)
				return err
			}
			if len(veniceEndpoints) == 0 {
				break L
			}
			time.Sleep(2 * time.Second)
		}
	}

	for _, obj := range veniceNetworks {
		if err := rClient.DeleteNetwork(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}

	for _, obj := range veniceHosts {
		if err := rClient.DeleteHost(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}

	return nil
}

//PushConfig populate configuration
func (vc *VcenterCfg) PushConfig() error {

	configPushCheck := func(done chan error) {
		iter := 1
		for ; iter <= 1500; iter++ {
			//Check every second
			time.Sleep(time.Second * time.Duration(iter))
			complete, err := vc.IsConfigPushComplete()
			if complete && err == nil {
				done <- nil
			}
		}
		done <- fmt.Errorf("Config push incomplete")
	}

	var err error
	err = vc.CleanupAllConfig()
	if err != nil {
		return err
	}

	//if os.Getenv("USE_STAGING_BUFFER") != "" {
	//	err = vc.pushConfigViaStagingBuffer(ctx, urls)
	//} else {
	err = vc.pushConfigViaRest()
	//}

	if err != nil {
		return err
	}

	policyPushCheck := func(done chan error) {

		rClient := vc.Client

		for _, o := range vc.Cfg.ConfigItems.SGPolicies {
			// verify that sgpolicy object has reached all naples
			iter := 1
			for ; iter <= 2000; iter++ {
				time.Sleep(time.Second * time.Duration(iter))
				retSgp, err := rClient.GetNetworkSecurityPolicy(&o.ObjectMeta)
				if err != nil {
					done <- fmt.Errorf("error getting back policy %s %v", o.ObjectMeta.Name, err.Error())
					return
				} else if retSgp.Status.PropagationStatus.Updated == int32(len(vc.Dscs)-len(vc.ThirdPartyDscs)) {
					log.Infof("got back policy satus %+v", retSgp.Status.PropagationStatus)
					done <- nil
					return
				}
				log.Warnf("Propagation stats did not match for policy %v. %+v", o.ObjectMeta.Name, retSgp.Status.PropagationStatus)
			}
			done <- fmt.Errorf("unable to update policy '%s' on all naples %+v",
				o.ObjectMeta.Name, o.Status.PropagationStatus)
		}
	}
	doneChan := make(chan error, 2)
	go policyPushCheck(doneChan)
	go configPushCheck(doneChan)

	for i := 0; i < 2; i++ {
		retErr := <-doneChan
		if retErr != nil {
			err = retErr
		}
	}

	writeConfig := func() {
		ofile, err := os.OpenFile(configFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0755)
		if err != nil {
			panic(err)
		}
		j, err := json.MarshalIndent(&vc.Cfg.ConfigItems, "", "  ")
		ofile.Write(j)
		ofile.Close()
	}

	writeConfig()

	return nil
}

//Vcenter mode we don't allow to create workloads, so return from cache
func (vc *VcenterCfg) ListWorkload() (objs []*workload.Workload, err error) {

	wloads := []*workload.Workload{}
	for _, wl := range vc.Cfg.ConfigItems.Workloads {
		wloads = append(wloads, wl)
	}

	return wloads, nil
}

// GetWorkload returns venice workload by object meta
func (vc *VcenterCfg) GetWorkload(meta *api.ObjectMeta) (w *workload.Workload, err error) {

	for _, wl := range vc.Cfg.ConfigItems.Workloads {
		if wl.ObjectMeta.Name == meta.Name {
			return wl, nil
		}
	}

	return nil, errors.New("Did not find workload")
}

// CreateWorkloads creates workloads
func (vc *VcenterCfg) CreateWorkloads(wrklds []*workload.Workload) error {
	log.Infof("Skipping creation of workload in vcenter.")
	return nil
}

// DeleteWorkloads creates workloads
func (vc *VcenterCfg) DeleteWorkloads(wrklds []*workload.Workload) error {
	log.Infof("Skipping deletion of workload in vcenter.")
	return nil
}
