// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package objstoreinteg

import (
	"bytes"
	"fmt"
	"os/exec"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/testenv"

	"testing"

	"context"
	"flag"
	"io/ioutil"
	"reflect"

	. "github.com/pensando/sw/venice/utils/testutils"
)

// integ test suite parameters
const (
	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath  = "../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../venice/utils/certmgr/testdata/roots.pem"
)

// objstoreIntegSuite is the state of integ test
type objstoreIntegSuite struct {
	certSrv        *certsrv.CertSrv
	restClient     apiclient.Services
	resolverSrv    *rpckit.RPCServer
	resolverClient resolver.Interface
}

func TestObjStoreInteg(t *testing.T) {
	// integ test suite
	var sts = &objstoreIntegSuite{}
	flag.Set("stderrthreshold", "INFO")
	flag.Parse()
	var _ = Suite(sts)
	TestingT(t)
}

func (it *objstoreIntegSuite) SetUpSuite(c *C) {
	// start certificate server
	certSrv, err := certsrv.NewCertSrv("localhost:0", certPath, keyPath, rootsPath)
	c.Assert(err, IsNil)
	it.certSrv = certSrv
	log.Infof("Created cert endpoint at %s", globals.CMDCertAPIPort)

	// instantiate a CKM-based TLS provider and make it default for all rpckit clients and servers
	tlsProvider := func(svcName string) (rpckit.TLSProvider, error) {
		p, err := tlsproviders.NewDefaultCMDBasedProvider(certSrv.GetListenURL(), svcName)
		if err != nil {
			return nil, err
		}
		return p, nil
	}
	testenv.EnableRpckitTestMode()
	rpckit.SetTestModeDefaultTLSProvider(tlsProvider)

	// Now create a mock resolver
	m := mock.NewResolverService()
	resolverHandler := service.NewRPCHandler(m)
	resolverServer, err := rpckit.NewRPCServer(globals.Cmd, "localhost:0", rpckit.WithTracerEnabled(true))
	c.Assert(err, IsNil)
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)
	resolverServer.Start()
	it.resolverSrv = resolverServer

	// start objstore
	cmd := []string{
		"run",
		"-d",
		"-p",
		"19001:19001",
		"-e",
		"MINIO_ACCESS_KEY=miniokey",
		"-e",
		"MINIO_SECRET_KEY=minio0523",
		"--name",
		"objstore",
		"registry.test.pensando.io:5000/objstore:v0.2",
		"server",
		"/data",
	}
	_, err = exec.Command("docker", cmd...).CombinedOutput()
	AssertOk(c, err, fmt.Sprintf("failed to start objstore, %s", err))

	// populate the mock resolver with apiserver instance.
	apiSrvSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "objstore1",
		},
		Service: globals.ObjStore,
		Node:    "localhost",
		URL:     "127.0.0.1:19001",
	}
	m.AddServiceInstance(&apiSrvSi)

	// create a controller
	rc := resolver.New(&resolver.Config{Name: globals.Npm, Servers: []string{resolverServer.GetListenURL()}})
	it.resolverClient = rc
}

func (it *objstoreIntegSuite) SetUpTest(c *C) {
	log.Infof("============================= %s starting ==========================", c.TestName())
}

func (it *objstoreIntegSuite) TearDownTest(c *C) {
	log.Infof("============================= %s completed ==========================", c.TestName())

}

func (it *objstoreIntegSuite) TearDownSuite(c *C) {
	// stop server and client
	log.Infof("Stop all Test Controllers")
	it.certSrv.Stop()

	it.resolverClient.Stop()
	it.resolverSrv.Stop()

	cmd := []string{
		"rm", "-f", "objstore",
	}
	_, err := exec.Command("docker", cmd...).CombinedOutput()
	AssertOk(c, err, fmt.Sprintf("failed to remove objstore, %s", err))

}

// basic test to make sure all components come up
func (it *objstoreIntegSuite) TestObjStoreApis(c *C) {
	oc, err := objstore.NewClient("default", "pktcap", it.resolverClient)
	AssertOk(c, err, fmt.Sprintf("obj store client failed"))
	_, err = oc.ListObjects("")
	AssertOk(c, err, fmt.Sprintf("list objects failed"))

	obj1Data := []byte("test object")
	b := bytes.NewBuffer(obj1Data)
	meta := map[string]string{
		"Key1": "val1",
		"Key2": "val2",
		"Key3": "val3",
		"Key4": "val4",
		"Key5": "val5",
	}

	// put obj
	n, err := oc.PutObject(context.Background(), "obj1", b, meta)
	AssertOk(c, err, fmt.Sprintf("put object failed"))
	Assert(c, n > 0, fmt.Sprintf("put object returned invalid bytes"))

	// stat obj
	s, err := oc.StatObject("obj1")
	AssertOk(c, err, fmt.Sprintf("stat object failed"))
	for k, v := range meta {
		Assert(c, s.MetaData[k] == v, fmt.Sprintf("meta didn't match key %s in %+v, expected %+v", k, s.MetaData, v))
	}

	// list obj
	objList, err := oc.ListObjects("obj1")
	AssertOk(c, err, fmt.Sprintf("list object failed"))
	Assert(c, len(objList) == 1, fmt.Sprintf("too many objects in list %+v", objList))

	// get obj
	reader, err := oc.GetObject(context.Background(), "obj1")
	AssertOk(c, err, fmt.Sprintf("get object failed"))
	data, err := ioutil.ReadAll(reader)
	AssertOk(c, err, fmt.Sprintf("read object failed"))
	Assert(c, reflect.DeepEqual(data, obj1Data), fmt.Sprintf("invalid object read :%s, expected: %s", string(data), string(obj1Data)))

	// rem obj
	err = oc.RemoveObjects("obj1")
	AssertOk(c, err, fmt.Sprintf("delete object failed"))
	objList, err = oc.ListObjects("obj1")
	AssertOk(c, err, fmt.Sprintf("list object failed"))
	Assert(c, len(objList) == 0, fmt.Sprintf("too many objects in list %+v", objList))

	// putstream
	wr, err := oc.PutStreamObject(context.Background(), "obj2", meta)
	AssertOk(c, err, fmt.Sprintf("putstream object failed"))
	Assert(c, wr != nil, fmt.Sprintf("put object returned invalid writer"))

	for i := 0; i < 10; i++ {
		s, err = oc.StatObject("obj2")
		AssertOk(c, err, fmt.Sprintf("stat on stream object failed"))

		n, err := wr.Write(obj1Data)
		AssertOk(c, err, fmt.Sprintf("write object failed"))
		Assert(c, n > 0, fmt.Sprintf("invalid bytes from write %+v", n))
	}
	objList, err = oc.ListObjects("obj2")
	AssertOk(c, err, fmt.Sprintf("list object failed"))
	Assert(c, len(objList) == 11, fmt.Sprintf("too many objects in list %+v", objList))
	wr.Close()
	objList, err = oc.ListObjects("obj2")
	AssertOk(c, err, fmt.Sprintf("list object failed"))
	Assert(c, len(objList) == 10, fmt.Sprintf("too many objects in list %+v", objList))

	// get stream
	for i := 0; i < 10; i++ {
		reader, err := oc.GetStreamObjectAtOffset(context.Background(), "obj2", i)
		AssertOk(c, err, fmt.Sprintf("getstream failed for %d", i))
		_, err = ioutil.ReadAll(reader)
		AssertOk(c, err, fmt.Sprintf("reader failed for %d", i))
	}

	// remove stream
	err = oc.RemoveObjects("obj2")
	AssertOk(c, err, fmt.Sprintf("delete object failed"))
	objList, err = oc.ListObjects("obj2")
	AssertOk(c, err, fmt.Sprintf("list object failed"))
	Assert(c, len(objList) == 0, fmt.Sprintf("too many objects in list %+v", objList))

}
