package services

import (
	"context"
	"fmt"
	"sync"
	"time"

	gogotypes "github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/installer"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/cmd/utils"
	rolloutproto "github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/imagestore"
	"github.com/pensando/sw/venice/utils/log"
)

type statusKey struct {
	op      rolloutproto.VeniceOp
	version string
}
type serviceStatusKey struct {
	op      rolloutproto.ServiceOp
	version string
}

type rolloutMgr struct {
	sync.RWMutex
	statusWriter        types.VeniceRolloutStatusWriter
	serviceStatusWriter types.ServiceRolloutStatusWriter
	cachedStatus        map[statusKey]rolloutproto.VeniceOpStatus
	cachedServiceStatus map[serviceStatusKey]rolloutproto.ServiceOpStatus
	upgradeServices     func([]string) error
	getUpgradeOrder     func() []string
	venicePreCheck      func(version string) error
	veniceRunVersion    func(version string) error
}

const syncDelaySeconds = 240

var serviceSyncDelaySeconds = syncDelaySeconds * time.Second

func newRolloutMgr() *rolloutMgr {
	r := rolloutMgr{
		cachedStatus:        make(map[statusKey]rolloutproto.VeniceOpStatus),
		cachedServiceStatus: make(map[serviceStatusKey]rolloutproto.ServiceOpStatus),
		getUpgradeOrder:     utils.GetUpgradeOrder,
		venicePreCheck:      installer.PreCheck,
		veniceRunVersion:    installer.RunVersion,
	}
	if env.K8sService != nil {
		r.upgradeServices = env.K8sService.UpgradeServices
	}
	return &r
}

// NewRolloutMgr returns RolloutMgr
func NewRolloutMgr() types.RolloutMgr {
	return newRolloutMgr()
}

// Start starts the rolloutMgr
func (r *rolloutMgr) Start() {
	r.Lock()
	defer r.Unlock()

}

// Stop stops the rolloutMgr
func (r *rolloutMgr) Stop() {
	r.Lock()
	defer r.Unlock()
}

func (r *rolloutMgr) CreateVeniceRollout(ro *rolloutproto.VeniceRollout) {
	r.handleVeniceRollout(ro)
}

func (r *rolloutMgr) doOP(op rolloutproto.VeniceOp, version string) rolloutproto.VeniceOpStatus {
	st := rolloutproto.VeniceOpStatus{
		Op:      op,
		Version: version,
	}
	bypassVeniceRollout := utils.GetConfigProperty("venice/cmd/bypassVeniceRollout")
	if bypassVeniceRollout != "" {
		st.OpStatus = "success"
		st.Message = "Bypassed due to custom config"
		log.Debugf("bypassed venice node upgrade to version %s based on custom config", version)
		return st
	}

	var err error
	switch op {
	case rolloutproto.VeniceOp_VenicePreCheck:
		err = r.venicePreCheck(version)
		log.Debugf("Done with Preload image")
	case rolloutproto.VeniceOp_VeniceRunVersion:
		err = r.veniceRunVersion(version)
		log.Debugf("Done with Install/Run image")
	}

	if err != nil {
		st.OpStatus = "failure"
		st.Message = err.Error()
		return st
	}

	st.OpStatus = "success"
	return st

}
func (r *rolloutMgr) handleVeniceRollout(ro *rolloutproto.VeniceRollout) {
	var opStatus []rolloutproto.VeniceOpStatus
	var opSpec rolloutproto.VeniceOpSpec
	needtoUpdateStatus := false

	for _, opSpec = range ro.Spec.Ops {
		key := statusKey{
			op:      opSpec.Op,
			version: opSpec.Version,
		}

		existingStatus, found := r.cachedStatus[key]
		if !found {
			var st rolloutproto.VeniceOpStatus
			var veniceVersion string
			var err error
			if env.ResolverClient == nil {
				veniceVersion = opSpec.Version
			} else {
				veniceVersion, err = imagestore.GetVeniceRolloutVersion(context.TODO(), env.ResolverClient, opSpec.Version)
				if err != nil {
					log.Errorf("Failed to get veniceVersion from objectStore %+v", err)
					veniceVersion = opSpec.Version
					//continue
				}
			}
			cmdVersion := utils.GetGitVersion()
			log.Infof("Rollout from %+v to bundleVersion %+v and veniceVersion %+v : %+v", env.GitVersion, opSpec.Version, veniceVersion, cmdVersion)
			if env.GitVersion != cmdVersion[veniceVersion] {
				log.Infof("Performing Rollout from version %+v", utils.GetGitVersion())
				st = r.doOP(opSpec.Op, opSpec.Version)
			} else {
				log.Info("CMD cameup fine. Setting the  veniceRolloutStatus to Success")
				st = rolloutproto.VeniceOpStatus{
					Op:      opSpec.Op,
					Version: opSpec.Version,
				}
				st.OpStatus = "success"
			}
			opStatus = append(opStatus, st)
			r.cachedStatus[key] = st
			needtoUpdateStatus = true
		} else {
			opStatus = append(opStatus, existingStatus)
		}
	}

	if r.statusWriter != nil && needtoUpdateStatus {
		s := rolloutproto.VeniceRolloutStatusUpdate{
			ObjectMeta: ro.ObjectMeta,
			Status: rolloutproto.VeniceRolloutStatus{
				OpStatus: opStatus,
			},
		}
		if opSpec.Op == rolloutproto.VeniceOp_VeniceRunVersion && utils.IsRunningOnVeniceAppl() {
			//Wait long enough for citadel to sync-data
			log.Infof(" Waiting long enough (4mins) for services to comeup")
			time.Sleep(serviceSyncDelaySeconds)
		}
		log.Infof(" Writing venice rollout status :%#v ", s)
		r.statusWriter.WriteStatus(context.TODO(), &s)
	}
}

func (r *rolloutMgr) UpdateVeniceRollout(ro *rolloutproto.VeniceRollout) {
	r.handleVeniceRollout(ro)
}

func (r *rolloutMgr) DeleteVeniceRollout(ro *rolloutproto.VeniceRollout) {
	for key := range r.cachedStatus {
		delete(r.cachedStatus, key)
	}
}

func (r *rolloutMgr) RegisterVeniceStatusWriter(statusWriter types.VeniceRolloutStatusWriter) {
	r.Lock()
	if r.statusWriter != statusWriter {
		r.statusWriter = statusWriter
	}
	r.Unlock()
}

func (r *rolloutMgr) UnregisterVeniceStatusWriter(statusWriter types.VeniceRolloutStatusWriter) {
	r.Lock()
	r.statusWriter = nil
	r.Unlock()
}

func (r *rolloutMgr) CreateServiceRollout(ro *rolloutproto.ServiceRollout) {
	r.handleServiceRollout(ro)
}

func (r *rolloutMgr) doServiceOP(op rolloutproto.ServiceOp, version string) rolloutproto.ServiceOpStatus {
	st := rolloutproto.ServiceOpStatus{
		Op:      op,
		Version: version,
	}
	bypassServiceRollout := utils.GetConfigProperty("venice/cmd/bypassServiceRollout")
	if bypassServiceRollout != "" {
		st.OpStatus = "success"
		st.Message = "Bypassed due to custom config"
		log.Debugf("bypassed upgrade services to version %s based on custom config", version)
		return st
	}

	switch op {
	case rolloutproto.ServiceOp_ServiceRunVersion:
		log.Debugf("About to start upgrade services to version %s", version)
		// read the file for the updated list of services
		svcOrder := r.getUpgradeOrder()
		err := r.upgradeServices(svcOrder)
		if err != nil {
			st.OpStatus = "failure"
			msg := fmt.Sprintf("upgradeServices to version %s returned %v", version, err)
			st.Message = msg
			log.Infof(msg)
			return st
		}
		log.Debugf("completed upgrade services to version %s", version)
	}
	st.OpStatus = "success"
	return st

}
func (r *rolloutMgr) handleServiceRollout(ro *rolloutproto.ServiceRollout) {
	var opStatus []rolloutproto.ServiceOpStatus
	needtoUpdateStatus := false

	versionIf := env.CfgWatcherService.APIClient().Version()
	clusterVersion, err := versionIf.Get(context.Background(), &api.ObjectMeta{Name: globals.DefaultVersionName})
	if err != nil {
		log.Infof("Failed to get ClusterVersion from apiserver %+v. continuing..", err)
		clusterVersion = &cluster.Version{}
	}

	for _, opSpec := range ro.Spec.Ops {
		if clusterVersion.Status.BuildVersion == opSpec.Version {
			opStatus = append(opStatus, rolloutproto.ServiceOpStatus{
				Op:       opSpec.Op,
				Version:  opSpec.Version,
				OpStatus: "success",
			})
			needtoUpdateStatus = true
			continue
		}

		key := serviceStatusKey{
			op:      opSpec.Op,
			version: opSpec.Version,
		}

		existingStatus, found := r.cachedServiceStatus[key]
		if !found {
			st := r.doServiceOP(opSpec.Op, opSpec.Version)
			opStatus = append(opStatus, st)
			r.cachedServiceStatus[key] = st
			needtoUpdateStatus = true
		} else {
			opStatus = append(opStatus, existingStatus)
		}
	}
	if r.serviceStatusWriter != nil && needtoUpdateStatus {
		s := rolloutproto.ServiceRolloutStatusUpdate{
			ObjectMeta: ro.ObjectMeta,
			Status: rolloutproto.ServiceRolloutStatus{
				OpStatus: opStatus,
			},
		}
		//Wait long enough for citadel to sync-data
		log.Infof(" Waiting long enough(4mins) for services to sync data")
		time.Sleep(serviceSyncDelaySeconds)
		log.Infof(" Writing service rollout status :%#v ", s)
		r.serviceStatusWriter.WriteServiceStatus(context.TODO(), &s)
	}

	log.Infof(" Cluster version Info %v", clusterVersion)
	c, _ := gogotypes.TimestampProto(time.Now())
	if env.CfgWatcherService != nil && clusterVersion.Status.BuildVersion != env.GitVersion {
		clusterVersion = &cluster.Version{
			TypeMeta: api.TypeMeta{
				Kind:       "Version",
				APIVersion: "v1",
			},
			ObjectMeta: api.ObjectMeta{
				Name:         globals.DefaultVersionName,
				CreationTime: clusterVersion.CreationTime,
				ModTime:      api.Timestamp{Timestamp: *c},
			},
			Spec: cluster.VersionSpec{},
			Status: cluster.VersionStatus{
				BuildVersion:        env.GitVersion,
				VCSCommit:           env.GitCommit,
				BuildDate:           env.BuildDate,
				RolloutBuildVersion: "",
			},
		}

		versionIf := env.CfgWatcherService.APIClient().Version()
		_, err := versionIf.UpdateStatus(context.Background(), clusterVersion)
		if err != nil {
			log.Errorf("Failed to update ClusterVersion to objectStore %+v", err)
		}
	}
}

func (r *rolloutMgr) UpdateServiceRollout(ro *rolloutproto.ServiceRollout) {
	r.handleServiceRollout(ro)
}

func (r *rolloutMgr) DeleteServiceRollout(ro *rolloutproto.ServiceRollout) {
	for key := range r.cachedServiceStatus {
		delete(r.cachedServiceStatus, key)
	}
}

func (r *rolloutMgr) RegisterServiceStatusWriter(statusWriter types.ServiceRolloutStatusWriter) {
	r.Lock()
	if r.serviceStatusWriter != statusWriter {
		r.serviceStatusWriter = statusWriter
	}
	r.Unlock()
}

func (r *rolloutMgr) UnregisterServiceStatusWriter(statusWriter types.ServiceRolloutStatusWriter) {
	r.Lock()
	r.serviceStatusWriter = nil
	r.Unlock()
}
