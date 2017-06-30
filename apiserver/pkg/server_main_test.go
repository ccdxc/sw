package apisrvpkg

import (
	"bytes"
	"fmt"
	"os"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/utils/kvstore/memkv"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/runtime"
)

type TestType1 struct {
	api.TypeMeta
	api.ObjectMeta
	A string
}

type TestType2 struct {
	api.TypeMeta
	api.ObjectMeta
	A int
}

func TestMain(m *testing.M) {
	// Setup the Apiserver Singleton.
	fmt.Printf("Setting up test main")
	_ = MustGetAPIServer()
	buf := &bytes.Buffer{}
	l := log.GetNewLogger(false).SetOutput(buf)
	singletonAPISrv.version = "v1"
	singletonAPISrv.Logger = l
	s := runtime.NewScheme()
	s.AddKnownTypes(&TestType1{}, &TestType2{})
	var err error
	singletonAPISrv.kv, err = memkv.NewMemKv(nil, runtime.NewJSONCodec(s))
	if err != nil {
		panic("Failed to intialize KV store")
	}
	rcode := m.Run()
	//fmt.Printf("Test Logs == \n %v\n", buf.String())
	os.Exit(rcode)
}
