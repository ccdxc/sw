package audit

import (
	"context"
	"errors"
	"fmt"
	"sync"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/watcher"
)

const (
	policyKey = "Singleton"
)

var gPolicyGetter *policyGetter
var once sync.Once

type policyGetter struct {
	sync.RWMutex
	cache        *memdb.Memdb
	watcher      *watcher.Watcher
	logger       log.Logger
	resolver     resolver.Interface
	name         string // module name using the watcher
	apiServerURL string // api server address
	stopped      bool   // flag to determine it is ready to be re-initialized
}

// GetAuditPolicy returns audit policy
func (pg *policyGetter) GetAuditPolicy(tenant string) (*monitoring.AuditPolicy, error) {
	objMeta := &api.ObjectMeta{Name: policyKey, Tenant: tenant, Namespace: globals.DefaultNamespace}
	val, err := pg.cache.FindObject(string(monitoring.KindAuditPolicy), objMeta)
	if err != nil {
		pg.logger.ErrorLog("method", "GetAuditPolicy", "msg", fmt.Sprintf("AuditPolicy [%+v] not found in AuditPolicy cache, Err: %v", objMeta, err))
		b := balancer.New(pg.resolver)
		defer b.Close()
		apicl, err := apiclient.NewGrpcAPIClient(pg.name, pg.apiServerURL, pg.logger, rpckit.WithBalancer(b))
		if err != nil {
			pg.logger.ErrorLog("method", "GetAuditPolicy", "msg", fmt.Sprintf("Error connecting to gRPC server [%s]: %v", pg.apiServerURL, err))
			return nil, err
		}
		defer apicl.Close()
		policy, err := apicl.MonitoringV1().AuditPolicy().Get(context.Background(), objMeta)
		if err != nil {
			// we don't mandate creation of audit policy; return an empty policy
			pg.logger.InfoLog("method", "GetAuditPolicy", "msg", fmt.Sprintf("no audit policy for [%s|%s]: %v", objMeta.Tenant, objMeta.Name, err))
			policy = &monitoring.AuditPolicy{}
			policy.Defaults("all")
		}
		policy.Name = policyKey
		policy.Tenant = tenant
		err = pg.cache.AddObject(policy)
		if err != nil {
			pg.logger.ErrorLog("Error adding audit policy [%s|%s] to cache: %v", objMeta.Tenant, objMeta.Name, err)
			return nil, err
		}
		val = policy
	}
	policy, ok := val.(*monitoring.AuditPolicy)
	if !ok {
		pg.logger.ErrorLog("method", "GetAuditPolicy", "msg", fmt.Sprintf("Invalid AuditPolicy type found in cache: %+v", policy))
		pg.cache.DeleteObject(&monitoring.AuditPolicy{TypeMeta: api.TypeMeta{Kind: string(monitoring.KindAuditPolicy)}, ObjectMeta: *objMeta})
		return nil, errors.New("invalid object type in audit policy cache")
	}
	retPolicy := &monitoring.AuditPolicy{}
	policy.Clone(retPolicy)
	return retPolicy, nil
}

func (pg *policyGetter) ListAuditPolicies() []*monitoring.AuditPolicy {
	var policies []*monitoring.AuditPolicy
	objs := pg.cache.ListObjects(string(monitoring.KindAuditPolicy), nil)
	for _, obj := range objs {
		policy := obj.(*monitoring.AuditPolicy)
		policies = append(policies, policy)
	}
	return policies
}

func (pg *policyGetter) WatchAuditPolicy() (*memdb.Watcher, error) {
	watcher := &memdb.Watcher{
		Name:    fmt.Sprintf("%s-%s", "AuditPolicy", uuid.NewV4().String()),
		Channel: make(chan memdb.Event, memdb.WatchLen),
	}
	if err := pg.cache.WatchObjects(string(monitoring.KindAuditPolicy), watcher); err != nil {
		return nil, err
	}
	return watcher, nil
}

func (pg *policyGetter) StopWatchAuditPolicy(watcher *memdb.Watcher) error {
	return pg.cache.StopWatchObjects(string(monitoring.KindAuditPolicy), watcher)
}

// Stop un-initializes PolicyGetter
func (pg *policyGetter) Stop() {
	defer pg.Unlock()
	pg.Lock()
	pg.watcher.Stop()
	pg.stopped = true
}

// Start re-initializes PolicyGetter. It blocks if PolicyGetter has not been un-initialized through Stop()
func (pg *policyGetter) Start() {
	pg.start(pg.name, pg.apiServerURL, pg.resolver)
}

func (pg *policyGetter) start(name, apiServer string, rslver resolver.Interface) {
	defer pg.Unlock()
	pg.Lock()
	if pg.stopped {
		pg.name = name
		pg.apiServerURL = apiServer
		pg.resolver = rslver
		pg.watcher.Start(pg.name, pg.apiServerURL, pg.resolver)
		pg.stopped = false
	}
}

// no-op
func (pg *policyGetter) initiateWatchCb() {}

func (pg *policyGetter) processEventCb(evt *kvstore.WatchEvent) error {
	switch tp := evt.Object.(type) {
	case *monitoring.AuditPolicy:
		pg.processAuditPolicyEvent(evt, tp)
	default:
		pg.logger.ErrorLog("method", "processEventCb", "msg", fmt.Sprintf("watcher found object of invalid type: %+v", tp))
	}
	return nil
}

func (pg *policyGetter) processAuditPolicyEvent(evt *kvstore.WatchEvent, policyObj *monitoring.AuditPolicy) {
	policyObj.Name = policyKey
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		pg.cache.AddObject(policyObj)
		pg.logger.InfoLog("method", "processAuditPolicyEvent", "msg", "updated audit policy")
	}
}

// GetPolicyGetter returns a singleton implementation of PolicyGetter that caches AuditPolicy and keeps it in synch with API server
func GetPolicyGetter(name, apiServer string, rslver resolver.Interface, l log.Logger) PolicyGetter {
	module := name + ModuleSuffix
	if gPolicyGetter != nil {
		gPolicyGetter.start(name, apiServer, rslver)
	}
	once.Do(func() {
		if l == nil {
			l = log.GetNewLogger(log.GetDefaultConfig(module))
		}
		gPolicyGetter = &policyGetter{
			logger:       l,
			name:         module,
			apiServerURL: apiServer,
			resolver:     rslver,
			cache:        memdb.NewMemdb(),
			stopped:      false,
		}
		// Use a custom TLS client identity so that secret field is not zeroized by ApiServer on watch
		gPolicyGetter.watcher = watcher.NewWatcher(module, apiServer, rslver, l, gPolicyGetter.initiateWatchCb, gPolicyGetter.processEventCb,
			[]rpckit.Option{rpckit.WithTLSClientIdentity("audit-watcher")},
			&watcher.KindOptions{
				Kind:    string(monitoring.KindAuditPolicy),
				Options: &api.ListWatchOptions{},
			})
	})
	return gPolicyGetter
}
