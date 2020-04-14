// Package alertmgr creates and runs all components - watcher, policy engine, alert engine and exporter.
package alertmgr

import (
	"context"
	"fmt"
	"reflect"
	"strings"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	apiservice "github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/alertmgr/alertengine"
	"github.com/pensando/sw/venice/ctrler/alertmgr/exporter"
	objectdb "github.com/pensando/sw/venice/ctrler/alertmgr/objdb"
	"github.com/pensando/sw/venice/ctrler/alertmgr/policyengine"
	"github.com/pensando/sw/venice/ctrler/alertmgr/watcher"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	aeutils "github.com/pensando/sw/venice/utils/alertengine"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	// Wait time between alertmgr retries.
	retryDelay = time.Second
	// Wait time between API server retries.
	apiSrvWaitIntvl = time.Second
	// Maximum number of API server retries.
	maxAPISrvRetries = 200
)

// Interface for alertmgr.
type Interface interface {
	// Run alert manager in the main goroutine.
	Run()

	// Stop alert manager.
	Stop()

	// GetRunningStatus of alert manager.
	GetRunningStatus() bool
}

type mgr struct {
	// Alertmgr lock.
	sync.Mutex

	// AlertMgr context.
	ctx    context.Context
	cancel context.CancelFunc

	// Resolver for API server.
	rslvr resolver.Interface

	// AlertMgr logger.
	logger log.Logger

	// AlertMgr object db.
	objdb objectdb.Interface

	// AlertMgr API client
	apiClient apiservice.Services

	// Watcher, policy engine, alert engine, exporter.
	watcher      watcher.Interface
	policyEngine policyengine.Interface
	alertEngine  alertengine.Interface
	exporter     exporter.Interface

	// Running status of AlertMgr.
	running bool
}

// New instance of alert manager.
func New(logger log.Logger, rslvr resolver.Interface) (Interface, error) {
	// Create object db.
	objdb := objectdb.New()

	// Create watcher.
	w, err := watcher.New(objdb, logger)
	if err != nil {
		logger.Errorf("Failed to create watcher, err: %v", err)
		return nil, err
	}

	// Create policy engine.
	pe, err := policyengine.New(objdb, logger)
	if err != nil {
		logger.Errorf("Failed to create policy engine, err: %v", err)
		return nil, err
	}

	// Create alert engine.
	ae, err := alertengine.New(logger, rslvr, objdb)
	if err != nil {
		logger.Errorf("Failed to create alert engine, err: %v", err)
		return nil, err
	}

	// Create exporter.
	e, err := exporter.New(logger, rslvr)
	if err != nil {
		logger.Errorf("Failed to create exporter, err: %v", err)
		return nil, err
	}

	// Create context.
	ctx, cancel := context.WithCancel(context.Background())

	m := &mgr{
		ctx:          ctx,
		cancel:       cancel,
		rslvr:        rslvr,
		logger:       logger,
		objdb:        objdb,
		watcher:      w,
		policyEngine: pe,
		alertEngine:  ae,
		exporter:     e}

	logger.Infof("Created new alert manager")

	return m, nil
}

func (m *mgr) Run() {
	inited := false
	b := balancer.New(m.rslvr)

	for {
		if m.watcher.GetRunningStatus() {
			m.watcher.Stop()
		}

		if m.policyEngine.GetRunningStatus() {
			m.policyEngine.Stop()
		}

		if m.alertEngine.GetRunningStatus() {
			m.alertEngine.Stop()
		}

		if m.exporter.GetRunningStatus() {
			m.exporter.Stop()
		}

		if m.apiClient != nil {
			m.apiClient.Close()
		}

		if err := m.ctx.Err(); err != nil {
			return
		}

		// Create API client with resolver.
		apiClient, err := apiservice.NewGrpcAPIClient(globals.AlertMgr, globals.APIServer, m.logger, rpckit.WithBalancer(b))
		if err != nil {
			m.logger.Warnf("Failed to create api client, err: %v", err)
			time.Sleep(retryDelay)
			continue
		}

		m.apiClient = apiClient.(apiservice.Services)

		// Alertmgr might have just restarted.
		// While it restarted some reference objects might have gone away, leaving some alerts invalid.
		// Update the object db.
		if !inited {
			if err := m.init(); err != nil {
				m.logger.Warnf("Failed to initialize alertmgr, err: %v", err)
				time.Sleep(retryDelay)
				continue
			}
			inited = true
		}

		// Run watcher.
		wOutCh, wErrCh, err := m.watcher.Run(m.ctx, apiClient)
		if err != nil {
			m.logger.Warnf("Failed to run watcher, err: %v", err)
			time.Sleep(retryDelay)
			continue
		}

		// Run policy engine.
		peOutCh, peErrCh, err := m.policyEngine.Run(m.ctx, wOutCh)
		if err != nil {
			m.logger.Warnf("Failed to run policy engine, err: %v", err)
			time.Sleep(retryDelay)
			continue
		}

		// Run alert engine.
		aeOutCh, aeErrCh, err := m.alertEngine.Run(m.ctx, apiClient, peOutCh)
		if err != nil {
			m.logger.Warnf("Failed to run alert engine, err: %v", err)
			time.Sleep(retryDelay)
			continue
		}

		// Run exporter.
		eErrCh, err := m.exporter.Run(m.ctx, aeOutCh)
		if err != nil {
			m.logger.Warnf("Failed to run exporter, err: %v", err)
			time.Sleep(retryDelay)
			continue
		}

		// Flag running status.
		m.setRunningStatus(true)
		m.logger.Infof("Started alert manager")

		// Wait for an error.
		var chErr error
		select {
		case chErr = <-wErrCh:
		case chErr = <-peErrCh:
		case chErr = <-aeErrCh:
		case chErr = <-eErrCh:
		case <-m.ctx.Done():
			chErr = m.ctx.Err()
		}
		m.logger.Warnf("Restarting engines %v", chErr)
		time.Sleep(retryDelay)
	}
}

func (m *mgr) Stop() {
	if m.GetRunningStatus() {
		m.cancel()
		m.setRunningStatus(false)
	}
}

func (m *mgr) GetRunningStatus() bool {
	m.Lock()
	defer m.Unlock()
	return m.running
}

func (m *mgr) setRunningStatus(status bool) {
	m.Lock()
	defer m.Unlock()
	m.running = status
}

type service interface {
	List(ctx context.Context, opts *api.ListWatchOptions) ([]interface{}, error)
}

func (m *mgr) init() error {
	ctx := m.ctx

	// Get all alert policies from KV store.
	opts := api.ListWatchOptions{FieldSelector: "spec.resource != event"} // TODO add tenant info
	policies, err := m.apiClient.MonitoringV1().AlertPolicy().List(ctx, &opts)
	if err != nil {
		m.logger.Errorf("Failed to list alert policies, err: %v", err)
		return err
	}

	// Add them to object db.
	for _, p := range policies {
		err := m.objdb.Add(p)
		if err != nil {
			m.logger.Errorf("Failed to add policy to objdb, err: %v", err)
			return err
		}
	}

	// Get all objects of the kinds referenced by alert policies.
	for _, p := range policies {
		r := p.Spec.Resource

		// Validate the kind the policy references.
		policyResourceValid := func() bool {
			groupMap := runtime.GetDefaultScheme().Kinds()
			for group := range groupMap {
				if r == group {
					return true
				}
			}
			return false
		}()

		if !policyResourceValid {
			m.logger.Debugf("Invalid resource, %v, in policy %v", r, p)
			continue
		}

		apiKindMethods, err := func(apiClient apiservice.Services, kind string) ([]reflect.Value, error) {
			apiClientVal := reflect.ValueOf(apiClient)
			version := "V1"
			key := strings.Title(r) + version
			groupFunc := apiClientVal.MethodByName(key)
			if !groupFunc.IsValid() {
				err := fmt.Errorf("Invalid API group %s", key)
				m.logger.Errorf("Invalid API group %s", key)
				return nil, err
			}

			return groupFunc.Call(nil), nil
		}(m.apiClient, r)

		if err != nil {
			return err
		}

		opts := api.ListWatchOptions{} // TODO add tenant info
		objs, err := apiKindMethods[0].Interface().(service).List(ctx, &opts)
		if err != nil {
			m.logger.Errorf("Failed to list objects of kind %v, err %v", r, err)
			return err
		}

		// Add the objects to db.
		for _, obj := range objs {
			err := m.objdb.Add(obj.(objectdb.Object))
			if err != nil {
				m.logger.Errorf("Failed to add object to objdb, error %v", err)
				return err
			}
		}
	}

	// Get all alerts from KV store.
	opts = api.ListWatchOptions{} // TODO add tenant info
	alerts, err := m.apiClient.MonitoringV1().Alert().List(ctx, &opts)
	if err != nil {
		m.logger.Errorf("Failed to list alerts, err %v", err)
		return err
	}

	for _, alert := range alerts {
		err := m.objdb.Add(alert)
		if err != nil {
			m.logger.Errorf("Failed to add object to objdb, error %v", err)
			return err
		}

		var obj objectdb.Object
		var pol *monitoring.AlertPolicy

		// Check if the alert references are still valid.
		alertRefsIntact := func() bool {
			val := reflect.ValueOf(alert.Status.ObjectRef)
			obj = val.Interface().(objectdb.Object)
			if m.objdb.Find(obj.GetObjectKind(), obj.GetObjectMeta()) == nil {
				return false
			}

			pols := m.objdb.List("AlertPolicy")
			for _, p := range pols {
				pol = p.(*monitoring.AlertPolicy)
				polID := fmt.Sprintf("%s/%s", pol.GetName(), pol.GetUUID())
				if polID == alert.Status.Reason.PolicyID {
					return true
				}
			}
			return false
		}()

		if !alertRefsIntact {
			// Reference(s) do not exist.. delete alert from KV store.
			err := m.kvOp(alert, alertengine.AlertOpDelete)
			if err != nil {
				m.logger.Errorf("KV delete operation error %v", err)
				return err
			}

			err = m.objdb.Delete(alert)
			if err != nil {
				m.logger.Errorf("Failed to delete object from objdb, error %v", err)
				return err
			}
			continue
		}

		// Check if the referenced object still matches the referenced policy.
		policyMatchesObj, _ := aeutils.Match(pol.Spec.GetRequirements(), obj.(runtime.Object))
		if policyMatchesObj {
			if alert.Spec.State == monitoring.AlertState_RESOLVED.String() {
				err := m.kvOp(alert, alertengine.AlertOpReopen)
				if err != nil {
					m.logger.Errorf("KV update operation error %v", err)
					return err
				}

				err = m.objdb.Update(alert)
				if err != nil {
					m.logger.Errorf("Failed to update object in objdb, error %v", err)
					return err
				}
			}
		} else {
			if alert.Spec.State == monitoring.AlertState_OPEN.String() {
				err := m.kvOp(alert, alertengine.AlertOpResolve)
				if err != nil {
					m.logger.Errorf("KV update operation error %v", err)
					return err
				}

				err = m.objdb.Update(alert)
				if err != nil {
					m.logger.Errorf("Failed to delete update object in objdb, error %v", err)
					return err
				}
			}
		}
	}

	m.logger.Debugf("Initialized alerts manager")
	return nil
}

func (m *mgr) kvOp(alert *monitoring.Alert, op alertengine.AlertOp) error {
	var err error
	switch op {
	case alertengine.AlertOpReopen:
		alert.Spec.State = monitoring.AlertState_OPEN.String()
		now, _ := types.TimestampProto(time.Now())
		alert.ModTime = api.Timestamp{Timestamp: *now}
		_, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			_, err := m.apiClient.MonitoringV1().Alert().Update(ctx, alert)
			if err != nil {
				return false, err
			}
			return true, nil
		}, apiSrvWaitIntvl, maxAPISrvRetries)

	case alertengine.AlertOpResolve:
		alert.Spec.State = monitoring.AlertState_RESOLVED.String()
		now, _ := types.TimestampProto(time.Now())
		alert.Status.Resolved = &monitoring.AuditInfo{User: "System", Time: &api.Timestamp{Timestamp: *now}}
		alert.ModTime = api.Timestamp{Timestamp: *now}
		_, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			_, err := m.apiClient.MonitoringV1().Alert().Update(ctx, alert)
			if err != nil {
				return false, err
			}
			return true, nil
		}, apiSrvWaitIntvl, maxAPISrvRetries)

	case alertengine.AlertOpDelete:
		_, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			_, err := m.apiClient.MonitoringV1().Alert().Delete(ctx, alert.GetObjectMeta())
			if err != nil {
				return false, err
			}
			return true, nil
		}, apiSrvWaitIntvl, maxAPISrvRetries)
	}

	return err
}
