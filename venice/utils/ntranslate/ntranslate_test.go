package ntranslate

import (
	"fmt"
	"reflect"
	"strconv"
	"testing"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// implements `TranslatorFns` interface
type tstrFns struct {
	KeyToMetaF func(key interface{}) *api.ObjectMeta
	MetaToKeyF func(meta *api.ObjectMeta) interface{}
}

func (fns *tstrFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	return fns.KeyToMetaF(key)
}

func (fns *tstrFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	return fns.MetaToKeyF(meta)
}

func TestNTranslate(t *testing.T) {
	tstr := MustGetTranslator()
	Assert(t, tstr != nil, "failed to get translator")
	Assert(t, tstr.GetObjectMeta("SecurityPolicyKey", nil) == nil, "expected nil")
	Assert(t, tstr.GetKey("SecurityPolicyKey", nil) == nil, "expected nil")

	// register translator func against each kind
	// kind: SecurityPolicyKey
	spkFns := struct{ tstrFns }{}
	spkFns.KeyToMetaF = func(key interface{}) *api.ObjectMeta {
		spKey := key.(*halproto.SecurityPolicyKey)
		return &api.ObjectMeta{
			UUID: fmt.Sprintf("%d", spKey.SecurityPolicyId),
		}
	}
	spkFns.MetaToKeyF = func(meta *api.ObjectMeta) interface{} {
		id, err := strconv.ParseUint(meta.GetUUID(), 10, 64)
		if err != nil {
			return nil
		}
		return &halproto.SecurityPolicyKey{
			SecurityPolicyId: id,
		}
	}
	tstr.Register("SecurityPolicyKey", &spkFns)

	// kind: PortKeyHandle
	pkFns := struct{ tstrFns }{}
	pkFns.KeyToMetaF = func(key interface{}) *api.ObjectMeta {
		pKey := key.(*halproto.PortKeyHandle)
		return &api.ObjectMeta{UUID: fmt.Sprintf("%d", pKey.GetPortId())}
	}
	pkFns.MetaToKeyF = func(meta *api.ObjectMeta) interface{} {
		id, err := strconv.ParseUint(meta.GetUUID(), 10, 32)
		if err != nil {
			return nil
		}
		return &halproto.PortKeyHandle{
			KeyOrHandle: &halproto.PortKeyHandle_PortId{
				PortId: uint32(id),
			},
		}
	}
	tstr.Register("PortKeyHandle", &pkFns)

	// kind: CollectorKeyHandle (only key to meta is registered)
	ckFns := struct{ tstrFns }{}
	ckFns.KeyToMetaF = func(key interface{}) *api.ObjectMeta {
		cKey := key.(*halproto.CollectorKeyHandle)
		return &api.ObjectMeta{UUID: fmt.Sprintf("%d", cKey.GetCollectorId())}
	}
	ckFns.MetaToKeyF = func(meta *api.ObjectMeta) interface{} {
		return nil
	}
	tstr.Register("CollectorKeyHandle", &ckFns)

	// kind: RouteKeyHandle
	tstr.Register("RouteKeyHandle", nil)

	// run the tests; given the kind and object key, it should return *api.ObjectMeta
	tcs := []struct {
		kind            string
		objKey          proto.Message // hal or platform object key
		objMeta         *api.ObjectMeta
		expectedObjKey  proto.Message
		expectedObjMeta *api.ObjectMeta
	}{
		{
			kind:            "SecurityPolicyKey",
			objKey:          getObject(t, "SecurityPolicyKey"),
			objMeta:         &api.ObjectMeta{UUID: "102000"},
			expectedObjKey:  getObject(t, "SecurityPolicyKey"),
			expectedObjMeta: &api.ObjectMeta{UUID: "102000"},
		},
		{
			kind:            "PortKeyHandle",
			objKey:          getObject(t, "PortKeyHandle"),
			objMeta:         &api.ObjectMeta{UUID: "203000"},
			expectedObjKey:  getObject(t, "PortKeyHandle"),
			expectedObjMeta: &api.ObjectMeta{UUID: "203000"},
		},
		{
			kind:            "CollectorKeyHandle",
			objKey:          getObject(t, "CollectorKeyHandle"),
			objMeta:         &api.ObjectMeta{UUID: "304000"},
			expectedObjKey:  nil,
			expectedObjMeta: &api.ObjectMeta{UUID: "304000"},
		},
		{
			kind:            "RouteKeyHandle",
			objKey:          nil,
			objMeta:         nil,
			expectedObjKey:  nil,
			expectedObjMeta: nil,
		},
	}

	for _, tc := range tcs {
		// key to meta
		om := tstr.GetObjectMeta(tc.kind, tc.objKey)
		Assert(t, reflect.DeepEqual(om, tc.expectedObjMeta), "expected: %v, got: %v", tc.expectedObjMeta, om)

		// meta to key
		key := tstr.GetKey(tc.kind, tc.objMeta)
		Assert(t, reflect.DeepEqual(key, tc.expectedObjKey), "expected: %v, got: %v", tc.expectedObjKey, key)
	}

	// overwrite existing fns
	tstr.Register("CollectorKeyHandle", nil)
	om := tstr.GetObjectMeta("CollectorKeyHandle", nil)
	Assert(t, om == nil, "expected nil, got: %v", om)
}

func getObject(t *testing.T, kind string) proto.Message {
	switch kind {
	case "SecurityPolicyKey":
		return &halproto.SecurityPolicyKey{
			SecurityPolicyId: 102000,
		}
	case "PortKeyHandle":
		return &halproto.PortKeyHandle{
			KeyOrHandle: &halproto.PortKeyHandle_PortId{
				PortId: 203000,
			},
		}
	case "CollectorKeyHandle":
		return &halproto.CollectorKeyHandle{
			KeyOrHandle: &halproto.CollectorKeyHandle_CollectorId{
				CollectorId: 304000,
			},
		}
	}

	return nil
}
