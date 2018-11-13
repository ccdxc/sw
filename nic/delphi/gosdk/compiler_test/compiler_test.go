package compilertest

import (
	"testing"

	"github.com/golang/protobuf/proto"

	clientApi "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/gosdk/compiler_test/testproto"
	delphi "github.com/pensando/sw/nic/delphi/proto/delphi"
)

// A fake client to pass in to the message wrappers.
type testClient struct {
}

func (t *testClient) MountKind(kind string, mode delphi.MountMode) error {
	return nil
}

var lastMountKindKeyKeyValue = ""

func (t *testClient) MountKindKey(kind string, key string, mode delphi.MountMode) error {
	lastMountKindKeyKeyValue = key
	return nil
}

func (t *testClient) Run() {
}
func (t *testClient) IsConnected() bool {
	return true
}

func (t *testClient) SetObject(obj clientApi.BaseObject) error {
	return nil
}

func (t *testClient) DeleteObject(obj clientApi.BaseObject) error {
	return nil
}

func (*testClient) GetObject(kind string, key string) clientApi.BaseObject {
	return nil
}

func (t *testClient) WatchKind(kind string, reactor clientApi.BaseReactor) error {
	return nil
}

func (t *testClient) WatchMount(l clientApi.MountListener) error {
	return nil
}

var list = make([]clientApi.BaseObject, 0)

func (t *testClient) List(kind string) []clientApi.BaseObject {
	return list
}

func (t *testClient) Close() {

}

func (t *testClient) DumpSubtrees() {

}

type reactA struct {
}

func (r *reactA) OnMessageACreate(obj *testproto.MessageA) {

}

func (r *reactA) OnMessageAUpdate(old *testproto.MessageA, obj *testproto.MessageA) {

}

func (r *reactA) OnMessageADelete(obj *testproto.MessageA) {

}

// TestMessageA tests basic functionality, like setting and getting fields,
// creating the message, and making sure the actual protobuf message matches
// the values in the wrapper object
func TestMessageA(t *testing.T) {
	client := new(testClient)

	testproto.MessageAMount(client, delphi.MountMode_ReadWriteMode)
	testproto.MessageAMountKey(client, 1, delphi.MountMode_ReadWriteMode)
	if lastMountKindKeyKeyValue != "1" {
		t.Errorf("lastMountKindKeyKeyValue != 1 (%s)", lastMountKindKeyKeyValue)
	}

	a := testproto.MessageA{
		Key:         11,
		StringValue: "Empty",
	}
	a.Descriptor()   // just for coverage
	a.ProtoMessage() // just for coverage

	if a.GetMeta() != nil {
		t.Fail()
	}

	if a.GetDelphiMeta() != nil {
		t.Errorf(`a.GetDelphiMeta() != nil`)
	}

	if a.GetKey() != 11 {
		t.Fail()
	}

	if a.GetDelphiKey() != "11" {
		t.Fail()
	}

	if a.String() != `Key:11 StringValue:"Empty" ` {
		t.Fail()
	}

	if a.GetStringValue() != "Empty" {
		t.Fail()
	}

	a.Key = 12
	if a.GetDelphiKey() != "12" {
		t.Fail()
	}

	a.SetDelphiMeta(&delphi.ObjectMeta{
		Key: a.GetDelphiKey(),
	})
	if a.GetDelphiPath() != "MessageA|12" {
		t.Errorf(`a.GetPath() != "MessageA|12" -> %v`, a.GetDelphiPath())
	}

	data, err := proto.Marshal(&a)
	if err != nil {
		t.Errorf("%v", err)
	}

	base, err := testproto.MessageAFactory(client, data)
	if err != nil {
		t.Errorf("%v", err)
	}

	a2, ok := base.(*testproto.MessageA)
	if !ok {
		t.Errorf("a2, ok := base.(*MessageA_)")
	}

	if a2.Key != 12 {
		t.Errorf(`a2.Key() == 12`)
	}

	r := &reactA{}
	rl := make([]clientApi.BaseReactor, 1)
	rl[0] = r

	a2.TriggerEvent(client, nil, delphi.ObjectOperation_SetOp, rl)
	a2.TriggerEvent(client, a2, delphi.ObjectOperation_SetOp, rl)
	a2.TriggerEvent(client, a2, delphi.ObjectOperation_DeleteOp, rl)

	a.Reset() // just for coverage
}

type reactB struct {
}

func (r *reactB) OnMessageBCreate(obj *testproto.MessageB) {

}

func (r *reactB) OnMessageBUpdate(old *testproto.MessageB, obj *testproto.MessageB) {

}

func (r *reactB) OnMessageBDelete(obj *testproto.MessageB) {

}

// TestMessageB tests keys that are made of other objects
func TestMessageB(t *testing.T) {
	client := new(testClient)

	testproto.MessageBMount(client, delphi.MountMode_ReadWriteMode)
	testproto.MessageBMountKey(client, &testproto.MessageKey{Value: 12},
		delphi.MountMode_ReadWriteMode)
	if lastMountKindKeyKeyValue != "Value:12 " {
		t.Errorf("lastMountKindKeyKeyValue != \"Value:12 \" (%s)",
			lastMountKindKeyKeyValue)
	}

	b := testproto.MessageB{
		Key: &testproto.MessageKey{
			Value: 11,
		},
		StringValue: "Empty",
	}
	b.Descriptor()   // just for coverage
	b.ProtoMessage() // just for coverage

	if b.GetMeta() != nil {
		t.Fail()
	}

	if b.GetDelphiMeta() != nil {
		t.Errorf(`b.GetDelphiMeta() != nil`)
	}

	if b.GetKey().GetValue() != 11 {
		t.Fail()
	}

	if b.GetDelphiKey() != "Value:11 " {
		t.Errorf(`b.GetDelphiKey() != "Value:11 " -> %s`, b.GetDelphiKey())
	}

	if b.String() != `Key:<Value:11 > StringValue:"Empty" ` {
		t.Errorf("b.String() != `Key:11 StringValue:\"Empty\" -> %s", b.String())
	}

	if b.GetStringValue() != "Empty" {
		t.Errorf(`b.GetStringValue() != "Empty"`)
	}

	b.Key.Value = 12
	if b.GetDelphiKey() != "Value:12 " {
		t.Fail()
	}

	b.SetDelphiMeta(&delphi.ObjectMeta{
		Key: b.GetDelphiKey(),
	})
	if b.GetDelphiPath() != "MessageB|Value:12 " {
		t.Errorf(`b.GetPath() != "MessageB|Value:12 " -> %v`, b.GetDelphiPath())
	}

	data, err := proto.Marshal(&b)
	if err != nil {
		t.Errorf("%v", err)
	}

	base, err := testproto.MessageBFactory(client, data)
	if err != nil {
		t.Errorf("%v", err)
	}

	b2, ok := base.(*testproto.MessageB)
	if !ok {
		t.Errorf("b2, ok := base.(*MessageB)")
	}

	if b2.Key.Value != 12 {
		t.Errorf(`b2.Key.Value == 12`)
	}

	r := &reactB{}
	rl := make([]clientApi.BaseReactor, 1)
	rl[0] = r

	b2.TriggerEvent(client, nil, delphi.ObjectOperation_SetOp, rl)
	b2.TriggerEvent(client, b2, delphi.ObjectOperation_SetOp, rl)
	b2.TriggerEvent(client, b2, delphi.ObjectOperation_DeleteOp, rl)

	b.Reset() // just for coverage
	key := &testproto.MessageKey{}
	key.Descriptor()
	key.ProtoMessage()
	key.Reset()
}

type reactC struct {
}

func (r *reactC) OnMessageCCreate(obj *testproto.MessageC) {
}

func (r *reactC) OnMessageCUpdate(old *testproto.MessageC, obj *testproto.MessageC) {
}

func (r *reactC) OnMessageCDelete(obj *testproto.MessageC) {

}

// TestMessageC tests singleton
func TestMessageC(t *testing.T) {
	client := new(testClient)

	testproto.MessageCMount(client, delphi.MountMode_ReadWriteMode)

	c := testproto.MessageC{
		StringValue: "Empty",
	}
	c.Descriptor()   // just for coverage
	c.ProtoMessage() // just for coverage

	if c.GetMeta() != nil {
		t.Fail()
	}

	if c.GetDelphiMeta() != nil {
		t.Errorf(`c.GetDelphiMeta() != nil`)
	}

	if c.GetDelphiKey() != "default" {
		t.Errorf(`c.GetDelphiKey() != "default" -> %s`, c.GetDelphiKey())
	}

	if c.String() != `StringValue:"Empty" ` {
		t.Errorf("b.String() != `StringValue:\"Empty\" -> %s", c.String())
	}

	if c.GetStringValue() != "Empty" {
		t.Errorf(`b.GetStringValue() != "Empty"`)
	}

	c.SetDelphiMeta(&delphi.ObjectMeta{
		Key: c.GetDelphiKey(),
	})
	if c.GetDelphiPath() != "MessageC|default" {
		t.Errorf(`b.GetPath() != "MessageC|default" -> %v`, c.GetDelphiPath())
	}

	data, err := proto.Marshal(&c)
	if err != nil {
		t.Errorf("%v", err)
	}

	base, err := testproto.MessageCFactory(client, data)
	if err != nil {
		t.Errorf("%v", err)
	}

	c2, ok := base.(*testproto.MessageC)
	if !ok {
		t.Errorf("c2, ok := base.(*MessageC)")
	}

	r := &reactC{}
	rl := make([]clientApi.BaseReactor, 1)
	rl[0] = r

	c2.TriggerEvent(client, nil, delphi.ObjectOperation_SetOp, rl)
	c2.TriggerEvent(client, c2, delphi.ObjectOperation_SetOp, rl)
	c2.TriggerEvent(client, c2, delphi.ObjectOperation_DeleteOp, rl)

	c.Reset() // just for coverage
}
