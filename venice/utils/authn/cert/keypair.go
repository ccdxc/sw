package cert

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/tls"
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/watcher"
)

const (
	// AutoGenTLSCertEventMsg is the event message when auto generated TLS certificate is being used by API Gateway
	AutoGenTLSCertEventMsg = "API Gateway is using auto generated TLS certificate"
)

var gKeyPair *clusterKeyPair
var once sync.Once

type keycache struct {
	sync.RWMutex
	selfSigned *tls.Certificate // for recovery in case cert key pair stored in Cluster object is corrupted
	cert       *tls.Certificate // populated from saved cert key pair in Cluster obj
}

func (ks *keycache) getSelfSignedCert() *tls.Certificate {
	defer ks.RUnlock()
	ks.RLock()
	return ks.selfSigned
}

func (ks *keycache) setSelfSignedCert(cert *tls.Certificate) {
	defer ks.Unlock()
	ks.Lock()
	ks.selfSigned = cert
}

func (ks *keycache) getClusterCert() *tls.Certificate {
	defer ks.RUnlock()
	ks.RLock()
	return ks.cert
}

func (ks *keycache) setClusterCert(cert *tls.Certificate) {
	defer ks.Unlock()
	ks.Lock()
	ks.cert = cert
}

type clusterKeyPair struct {
	sync.RWMutex
	cache        keycache
	watcher      *watcher.Watcher
	logger       log.Logger
	resolver     resolver.Interface
	name         string // module name using the watcher
	apiServerURL string // api server address
	stopped      bool   // flag to determine it is ready to be re-initialized
}

func (kp *clusterKeyPair) generateSelfSignedCert() error {
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		return err
	}
	cert, err := certs.SelfSign(globals.APIGw, key, certs.WithValidityDays(3650))
	if err != nil {
		return err
	}
	kp.cache.setSelfSignedCert(&tls.Certificate{Certificate: [][]byte{cert.Raw}, PrivateKey: key})
	return nil
}

func (kp *clusterKeyPair) getCertificate() func(clientHello *tls.ClientHelloInfo) (*tls.Certificate, error) {
	return func(clientHello *tls.ClientHelloInfo) (*tls.Certificate, error) {
		cert := kp.cache.getClusterCert()
		if cert != nil {
			return cert, nil
		}
		return kp.cache.getSelfSignedCert(), nil
	}
}

func (kp *clusterKeyPair) TLSConfig() *tls.Config {
	return &tls.Config{
		GetCertificate: kp.getCertificate(),
	}
}

func (kp *clusterKeyPair) Start() {
	kp.start(kp.name, kp.apiServerURL, kp.resolver)
}

func (kp *clusterKeyPair) Stop() {
	defer kp.Unlock()
	kp.Lock()
	kp.watcher.Stop()
	kp.stopped = true
}

func (kp *clusterKeyPair) start(name, apiServer string, rslver resolver.Interface) {
	defer kp.Unlock()
	kp.Lock()
	if kp.stopped {
		kp.name = name
		kp.apiServerURL = apiServer
		kp.resolver = rslver
		kp.watcher.Start(kp.name, kp.apiServerURL, kp.resolver)
		kp.stopped = false
	}
}

// no-op
func (kp *clusterKeyPair) initiateWatchCb() {}

func (kp *clusterKeyPair) processEventCb(evt *kvstore.WatchEvent) error {
	switch tp := evt.Object.(type) {
	case *cluster.Cluster:
		kp.processClusterEvent(evt, tp)
	default:
		kp.logger.ErrorLog("method", "processEventCb", "msg", fmt.Sprintf("watcher found object of invalid type: %+v", tp))
	}
	return nil
}

func (kp *clusterKeyPair) processClusterEvent(evt *kvstore.WatchEvent, clusterObj *cluster.Cluster) {
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		cert, err := tls.X509KeyPair([]byte(clusterObj.Spec.Certs), []byte(clusterObj.Spec.Key))
		if err != nil {
			kp.logger.ErrorLog("method", "processClusterEvent", "msg", "error creating key pair", "error", err)
			if evt.Type == kvstore.Created {
				recorder.Event(eventtypes.AUTO_GENERATED_TLS_CERT, AutoGenTLSCertEventMsg, nil)
			}
			return
		}
		kp.cache.setClusterCert(&cert)
		kp.logger.InfoLog("method", "processClusterEvent", "msg", "updated key pair")
	case kvstore.Deleted:
		kp.cache.setClusterCert(nil)
		kp.logger.InfoLog("method", "processClusterEvent", "msg", "deleted key pair")
		recorder.Event(eventtypes.AUTO_GENERATED_TLS_CERT, AutoGenTLSCertEventMsg, nil)
	}
}

// GetKeyPair returns a singleton implementation of KeyPair that reads certificate and private key from Cluster object. If
// cert and key are absent in Cluster object then it falls back to using in-memory temporary self signed cert.
func GetKeyPair(name, apiServer string, rslver resolver.Interface, l log.Logger) KeyPair {
	// adding a suffix so that secret field TLS key is returned in cluster obj from API server on watch
	module := name + authn.ModuleSuffix
	if gKeyPair != nil {
		gKeyPair.start(name, apiServer, rslver)
	}
	once.Do(func() {
		gKeyPair = &clusterKeyPair{
			logger:       l,
			name:         module,
			apiServerURL: apiServer,
			resolver:     rslver,
		}
		gKeyPair.watcher = watcher.NewWatcher(module, apiServer, rslver, l, gKeyPair.initiateWatchCb, gKeyPair.processEventCb,
			&watcher.KindOptions{
				Kind:    string(cluster.KindCluster),
				Options: &api.ListWatchOptions{},
			})
		if err := gKeyPair.generateSelfSignedCert(); err != nil {
			gKeyPair.logger.Fatalf("error generating self signed cert: %v", err)
		}
	})
	return gKeyPair
}
