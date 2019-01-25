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
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/watcher"
)

var gKeyPair *clusterKeyPair
var once sync.Once

type clusterKeyPair struct {
	sync.RWMutex
	selfSigned   *tls.Certificate // for recovery in case cert key pair stored in Cluster object is corrupted
	cert         *tls.Certificate // populated from saved cert key pair in Cluster obj
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
	kp.Lock()
	kp.selfSigned = &tls.Certificate{
		Certificate: [][]byte{cert.Raw},
		PrivateKey:  key,
	}
	kp.Unlock()
	return nil
}

func (kp *clusterKeyPair) getCertificate() func(clientHello *tls.ClientHelloInfo) (*tls.Certificate, error) {
	return func(clientHello *tls.ClientHelloInfo) (*tls.Certificate, error) {
		defer kp.RUnlock()
		kp.RLock()
		if kp.cert == nil {
			return kp.selfSigned, nil
		}
		return kp.cert, nil
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

func (kp *clusterKeyPair) processEventCb(evt *kvstore.WatchEvent) {
	switch tp := evt.Object.(type) {
	case *cluster.Cluster:
		kp.processClusterEvent(evt, tp)
	default:
		kp.logger.ErrorLog("method", "processEventCb", "msg", fmt.Sprintf("watcher found object of invalid type: %+v", tp))
		return
	}
}

func (kp *clusterKeyPair) processClusterEvent(evt *kvstore.WatchEvent, clusterObj *cluster.Cluster) {
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		cert, err := tls.X509KeyPair([]byte(clusterObj.Spec.Certs), []byte(clusterObj.Spec.Key))
		if err != nil {
			kp.logger.ErrorLog("method", "processClusterEvent", "msg", "error creating key pair", "error", err)
			// TODO: Generate an alert self signed recovery cert will be used
			return
		}
		kp.Lock()
		kp.cert = &cert
		kp.Unlock()
		kp.logger.InfoLog("method", "processClusterEvent", "msg", "updated key pair")
	case kvstore.Deleted:
		kp.Lock()
		kp.cert = nil
		kp.Unlock()
		kp.logger.InfoLog("method", "processClusterEvent", "msg", "deleted key pair")
		// TODO: Generate an alert
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
