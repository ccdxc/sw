package basenetCfg

import (
	"errors"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/base"
	"github.com/pensando/sw/venice/utils/log"
)

const configFile = "/tmp/scale-cfg.json"

//VcenterCfg encapsulate all Vcenter configuration objects
type BasenetCfg struct {
	base.EntBaseCfg
}

//NewVcenterCfg new vcenter config
func NewBasenetCfg() *BasenetCfg {
	return &BasenetCfg{}
}

//CleanupAllConfig cleans up config which is independent
func (vc *BasenetCfg) CleanupAllConfig() error {
	var err error

	rClient := vc.Client

	// get all venice configs
	veniceHosts, err := rClient.ListHost()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
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

//IsConfigPushComplete checks whether config push is complete.
func (vc *BasenetCfg) IsConfigPushComplete() (bool, error) {

	return true, nil
}

//PushConfig populate configuration
func (vc *BasenetCfg) PushConfig() error {
	cfg := vc.Cfg

	rClient := vc.Client

	log.Infof("Creating hosts...")
	CreateHosts := func() error {
		for _, o := range cfg.ConfigItems.Hosts {
			err := rClient.CreateHost(o)
			if err != nil {
				log.Errorf("Error creating host: %+v. Err: %v", o, err)
				return err
			}
		}
		return nil
	}

	return CreateHosts()
}

//Basenet does have any config mode we don't allow to create workloads, so return from cache
func (vc *BasenetCfg) ListWorkload() (objs []*workload.Workload, err error) {

	wloads := []*workload.Workload{}
	for _, wl := range vc.Cfg.ConfigItems.Workloads {
		wloads = append(wloads, wl)
	}

	return wloads, nil
}

// GetWorkload returns venice workload by object meta
func (vc *BasenetCfg) GetWorkload(meta *api.ObjectMeta) (w *workload.Workload, err error) {

	for _, wl := range vc.Cfg.ConfigItems.Workloads {
		if wl.ObjectMeta.Name == meta.Name {
			return wl, nil
		}
	}

	return nil, errors.New("Did not find workload")
}

// CreateWorkloads creates workloads
func (vc *BasenetCfg) CreateWorkloads(wrklds []*workload.Workload) error {
	log.Infof("Skipping creation of workload in basenet.")
	return nil
}

// DeleteWorkloads creates workloads
func (vc *BasenetCfg) DeleteWorkloads(wrklds []*workload.Workload) error {
	log.Infof("Skipping deletion of workload in basenet.")
	return nil
}
