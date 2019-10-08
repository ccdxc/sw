package instanceManager

import (
	"context"
	"fmt"
	"net/url"
	"sync"
	"time"

	"github.com/vmware/govmomi/vim25/soap"

	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/orch/vchub/defs"
	"github.com/pensando/sw/venice/orch/vchub/vcprobe"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const watcherQueueLen = 1000

type syncFlag struct {
	sync.RWMutex
	flag bool
}

// InstanceManager is the struct which watches for update to vc config objects
type InstanceManager struct {
	waitGrp               sync.WaitGroup
	watchCtx              context.Context
	watchCancel           context.CancelFunc
	stopFlag              syncFlag
	orchestrationConfigCh chan orchestration.Orchestrator
	vcenterList           []*url.URL
	apisrvURL             string
	resolver              resolver.Interface
	storeCh               chan defs.StoreMsg
	logger                log.Logger
	vcProbeMap            map[string]*vcprobe.VCProbe
}

// Stop stops the watcher
func (w *InstanceManager) Stop() {
	w.stop()
	w.watchCancel()
	w.waitGrp.Wait()
}

func (w *InstanceManager) stopped() bool {
	w.stopFlag.RLock()
	defer w.stopFlag.RUnlock()
	return w.stopFlag.flag
}

func (w *InstanceManager) stop() {
	w.stopFlag.Lock()
	w.stopFlag.flag = true
	w.stopFlag.Unlock()
}

// NewInstanceManager creates a new watcher
func NewInstanceManager(apisrvURL string, resolver resolver.Interface, storeCh chan defs.StoreMsg, vcenterList []*url.URL, logger log.Logger) (*InstanceManager, error) {
	if len(apisrvURL) == 0 {
		return nil, fmt.Errorf("API server URL is empty")
	}

	log.Infof("Got API Server URL. [%v]", apisrvURL)
	watchCtx, watchCancel := context.WithCancel(context.Background())

	instance := &InstanceManager{
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		stopFlag: syncFlag{
			flag: false,
		},
		vcenterList: vcenterList,
		storeCh:     storeCh,
		apisrvURL:   apisrvURL,
		logger:      logger,
		resolver:    resolver,
		vcProbeMap:  make(map[string]*vcprobe.VCProbe),
	}

	return instance, nil
}

// Start starts instance manager
func (w *InstanceManager) Start() {
	retryMap := make(map[string]*vcprobe.VCProbe)

	if len(w.vcenterList) > 0 {
		for _, u := range w.vcenterList {
			vcp := vcprobe.NewVCProbe(u.Hostname()+":"+u.Port(), u, w.storeCh)
			if vcp.Start() == nil {
				vcp.Run()
				w.vcProbeMap[u.String()] = vcp
			} else {
				log.Info("Stopping VCP")
				vcp.Stop()
				retryMap[u.String()] = vcp
			}
		}
	}

	for {
		if len(retryMap) == 0 {
			break
		}

		time.Sleep(time.Second)
		for u, v := range retryMap {
			if v.Start() == nil {
				v.Run()
				delete(retryMap, u)
				w.vcProbeMap[u] = v
			} else {
				log.Info("Stopping VCP")
				v.Stop()
			}
		}
	}

	go w.runApisrvWatcher()
}

func (w *InstanceManager) handleConfigEvent(evtType kvstore.WatchEventType, config *orchestration.Orchestrator) {
	switch evtType {
	case kvstore.Created, kvstore.Updated:

		vc := fmt.Sprintf("https://%s:%s@%s/sdk", config.Spec.Credentials.UserName, config.Spec.Credentials.Password, config.Spec.URI)
		vcURL, err := soap.ParseURL(vc)
		if err != nil {
			log.Errorf("Failed to parse VCenter URL [%v]. Err : %v", vc, err)
			return
		}

		vcp := vcprobe.NewVCProbe(vcURL.Hostname()+":"+vcURL.Port(), vcURL, w.storeCh)
		started := false
		log.Infof("Config item created. %v", config)

		for !started {
			err := vcp.Start()
			if err == nil {
				vcp.Run()
				w.vcProbeMap[config.Spec.URI] = vcp
				started = true
				log.Infof("VC Probe started for %v", config.Spec.URI)
			} else {
				log.Errorf("Failed to start VCP : %v", err)
			}

			time.Sleep(time.Second)
		}
	case kvstore.Deleted:
		log.Infof("Config item deleted. %v", config)
		vcp := w.vcProbeMap[config.Spec.URI]
		if vcp != nil {
			log.Infof("Stopping VC Probe for %v", config.Spec.URI)
			vcp.Stop()
		}
	}
}
