package compilertest

import (
	"testing"

	"github.com/golang/protobuf/descriptor"
	"github.com/golang/protobuf/proto"

	gosdk "github.com/pensando/sw/nic/delphi/gosdk"
	delphi "github.com/pensando/sw/nic/delphi/proto/delphi"
)

// A fake client to pass in to the message wrappers.
type testClient struct {
}

func (t *testClient) MountKind(kind string, mode delphi.MountMode) error {
	return nil
}

func (t *testClient) Dial() error {
	return nil
}

func (t *testClient) SetObject(obj gosdk.BaseObject) error {
	return nil
}

func (t *testClient) DeleteObject(obj gosdk.BaseObject) error {
	return nil
}

func (*testClient) GetObject(kind string, key string) gosdk.BaseObject {
	return nil
}

func (t *testClient) WatchKind(kind string, reactor gosdk.BaseReactor) error {
	return nil
}

func (t *testClient) Close() {

}

func (t *testClient) DumpSubtrees() {

}

type reactA struct {
}

func (r *reactA) OnMessageACreate(obj *MessageA) {

}

func (r *reactA) OnMessageAUpdate(obj *MessageA) {

}

func (r *reactA) OnMessageADelete(obj *MessageA) {

}

// TestMessageA tests basic functionality, like setting and getting fields,
// creating the message, and making sure the actual protobuf message matches
// the values in the wrapper object
func TestMessageA(t *testing.T) {
	client := new(testClient)

	a := NewMessageA(client)
	a.SetKey(12)
	if a.GetKey() != 12 || a.key != 12 {
		t.Fail()
	}
	a.SetStringValue("testA")
	if a.GetStringValue() != "testA" || a.stringValue != "testA" {
		t.Fail()
	}
	if a.GetKeyString() != "12" {
		t.Fail()
	}

	msg, ok := a.GetMessage().(*MessageA_)
	if !ok {
		t.Errorf("Cast failed")
	}
	if msg.Key != 12 {
		t.Errorf("Cast failed")
	}
	if msg.StringValue != "testA" {
		t.Errorf("Was expecting \"testA\" got: %s", msg.StringValue)
	}

	meta := a.GetMeta()
	meta.Key = a.GetKeyString()
	if a.GetMeta().Key != "12" {
		t.Errorf("meta.Key is %v", meta.Key)
	}

	if a.GetPath() != "MessageA|12" {
		t.Errorf(`a.GetPath() != "MessageA|12" -> %v`, a.GetPath())
	}

	if msg.GetKey() != 12 {
		t.Errorf(`msg.GetKey() != 12 -> %v`, msg.GetKey())
	}

	if msg.GetMeta() == nil {
		t.Errorf(`msg.GetMeta() == nil`)
	}

	if msg.GetStringValue() != "testA" {
		t.Errorf(`msg.GetStringValue() != "testA"`)
	}

	if msg.String() != `Meta:<Kind:"MessageA" Key:"12" > Key:12 StringValue:"testA" ` {
		t.Errorf(`msg.String() != Meta:<Kind:"MessageA" Key:"12" > Key:12 StringValue:"testA" `)
	}

	_, desc := descriptor.ForMessage(msg)
	if *desc.Name != "MessageA_" {
		t.Errorf(`*desc.Name != "MessageA_"`)
	}

	data, err := proto.Marshal(msg)
	if err != nil {
		t.Errorf("%v", err)
	}

	base, err := messageAFactory(client, data)
	if err != nil {
		t.Errorf("%v", err)
	}

	a2, ok := base.(*MessageA)
	if !ok {
		t.Errorf("a2, ok := base.(*MessageA_)")
	}

	if a2.GetKey() != 12 {
		t.Errorf(`a2.Key() == 12`)
	}

	r := new(reactA)
	rl := make([]gosdk.BaseReactor, 1)
	rl[0] = r

	a2.TriggerEvent(nil, delphi.ObjectOperation_SetOp, rl)
	a2.TriggerEvent(a2, delphi.ObjectOperation_SetOp, rl)
	a2.TriggerEvent(a2, delphi.ObjectOperation_DeleteOp, rl)

	msg.Reset()
	if msg.String() != "" {
		t.Errorf(`msg.String() != ""`)
	}

	if GetMessageA(client, 0) != nil {
		t.Errorf(`GetMessageA("", "") != nil`)
	}
}

type reactB struct {
}

func (r *reactB) OnMessageBCreate(obj *MessageB) {

}

func (r *reactB) OnMessageBUpdate(obj *MessageB) {

}

func (r *reactB) OnMessageBDelete(obj *MessageB) {

}

// TestMessageB tests keys that are made of other objects
func TestMessageB(t *testing.T) {
	client := new(testClient)

	b := NewMessageB(client)
	b.GetKey().SetValue(12)
	if b.GetKey().GetValue() != 12 {
		t.Errorf("b.GetKey().GetValue() error (%v)", b.GetKey().GetValue())
	}

	if b.GetKeyString() != "Value:12 " {
		t.Errorf("b.GetKeyString() error (%v)", b.GetKeyString())
	}

	if b.GetPath() != "MessageB|Value:12 " {
		t.Errorf(`b.GetPath() != "MessageB|Value:12 " -> %v`, b.GetPath())
	}
}

type reactC struct {
}

func (r *reactC) OnMessageCCreate(obj *MessageC) {

}

func (r *reactC) OnMessageCUpdate(obj *MessageC) {

}

func (r *reactC) OnMessageCDelete(obj *MessageC) {

}

// TestMessageC tests messages with array field of strings.
func TestMessageC(t *testing.T) {
	client := new(testClient)

	c := NewMessageC(client)
	arrWrapper := c.GetStringValue()
	if arrWrapper == nil {
		t.Errorf("arr is nil")
	}

	arrWrapper.Append("testC")
	v := arrWrapper.Get(0)
	if v != "testC" {
		t.Errorf("v should be \"testC\" it is: %s", v)
	}

	msg, ok := c.GetMessage().(*MessageC_)
	if !ok {
		t.Errorf("Cast to *MessageC_ failed")
	}
	if msg.StringValue[0] != "testC" {
		t.Errorf("msg.StringValue[0] is wrong (%s)", msg.StringValue[0])
	}

	arrWrapper.Append("should not be appended in msg1")
	if len(msg.StringValue) != 1 {
		t.Errorf("msg.StringValue lenght should be 1. is %d",
			len(msg.StringValue))
	}

	c2 := newMessageCFromMessage(msg)
	if c2.GetStringValue().Get(0) != "testC" {
		t.Errorf(`c2.GetStringValue().Get(1) != "testC"`)
	}

	_ = append(msg.StringValue, "should not reflect in msg2")
	msg2, _ := c.GetMessage().(*MessageC_)
	if len(msg2.StringValue) != 2 {
		t.Errorf(`len(msg2.StringValue) != 2 -> %d`, len(msg2.StringValue))
	}
}

// TestMessageBExtra add extra tests to MessageB, thare are already tested as
// part of MessageA to reach 75% coverage.
func TestMessageBExtra(t *testing.T) {
	client := new(testClient)

	b := NewMessageB(client)
	b.GetKey().SetValue(12)
	if b.GetKey().GetValue() != 12 || b.key.value != 12 {
		t.Fail()
	}
	b.SetStringValue("testB")
	if b.GetStringValue() != "testB" || b.stringValue != "testB" {
		t.Fail()
	}
	if b.GetKeyString() != "Value:12 " {
		t.Fail()
	}

	msg, ok := b.GetMessage().(*MessageB_)
	if !ok {
		t.Errorf("Cast failed")
	}
	if msg.Key.Value != 12 {
		t.Errorf(`msg.Key.Value != 12`)
	}
	if msg.StringValue != "testB" {
		t.Errorf(`msg.StringValue != "testB"`)
	}

	meta := b.GetMeta()
	meta.Key = b.GetKeyString()
	if b.GetMeta().Key != "Value:12 " {
		t.Errorf(`b.GetMeta().Key != "Value:12 "`)
	}

	if b.GetPath() != "MessageB|Value:12 " {
		t.Errorf(`b.GetPath() != "MessageB|Value:12 "`)
	}

	if msg.GetKey().GetValue() != 12 {
		t.Errorf(`msg.GetKey() != 12 -> %v`, msg.GetKey())
	}

	if msg.GetMeta() == nil {
		t.Errorf(`msg.GetMeta() == nil`)
	}

	if msg.GetStringValue() != "testB" {
		t.Errorf(`msg.GetStringValue() != "testB"`)
	}

	if msg.String() != `Meta:<Kind:"MessageB" Key:"Value:12 " > Key:<Value:12 > StringValue:"testB" ` {
		t.Errorf(`msg.String() != Meta:<Kind:"MessageB" Key:"Value:12 " > Key:<Value:12 > StringValue:"testB" `)
	}

	_, desc := descriptor.ForMessage(msg)
	if *desc.Name != "MessageB_" {
		t.Errorf(`*desc.Name != "MessageB_"`)
	}

	data, err := proto.Marshal(msg)
	if err != nil {
		t.Errorf("%v", err)
	}

	base, err := messageBFactory(client, data)
	if err != nil {
		t.Errorf("%v", err)
	}

	b2, ok := base.(*MessageB)
	if !ok {
		t.Errorf("b2, ok := base.(*MessageB_)")
	}

	if b2.GetKey().GetValue() != 12 {
		t.Errorf(`b2.Key().GetValue() == 12`)
	}

	r := new(reactB)
	rl := make([]gosdk.BaseReactor, 1)
	rl[0] = r

	b2.TriggerEvent(nil, delphi.ObjectOperation_SetOp, rl)
	b2.TriggerEvent(b2, delphi.ObjectOperation_SetOp, rl)
	b2.TriggerEvent(b2, delphi.ObjectOperation_DeleteOp, rl)

	msg.Key.Reset()
	msg.Reset()
	if msg.String() != "" {
		t.Errorf(`msg.String() != ""`)
	}

	key := NewMessageKey(nil)
	key.SetValue(1)
	if GetMessageB(client, key) != nil {
		t.Errorf(`GetMessageB("", "") != nil`)
	}
}

// TestMessageCExtra add extra tests to MessageB, thare are already tested as
// part of MessageA to reach 75% coverage.
func TestMessageCExtra(t *testing.T) {
	client := new(testClient)

	c := NewMessageC(client)
	c.SetKey(12)
	if c.GetKey() != 12 || c.key != 12 {
		t.Fail()
	}
	c.GetStringValue().Append("testC")
	if c.GetStringValue().Get(0) != "testC" ||
		c.stringValue.values[0] != "testC" {
		t.Fail()
	}
	if c.GetKeyString() != "12" {
		t.Fail()
	}

	msg, ok := c.GetMessage().(*MessageC_)
	if !ok {
		t.Errorf("Cast failed")
	}
	if msg.Key != 12 {
		t.Errorf("Cast failed")
	}
	if msg.StringValue[0] != "testC" {
		t.Errorf("Was expecting \"testC\" got: %s", msg.StringValue)
	}

	meta := c.GetMeta()
	meta.Key = c.GetKeyString()
	if c.GetMeta().Key != "12" {
		t.Errorf("meta.Key is %v", meta.Key)
	}

	if c.GetPath() != "MessageC|12" {
		t.Errorf(`a.GetPath() != "MessageA|12" -> %v`, c.GetPath())
	}

	if msg.GetKey() != 12 {
		t.Errorf(`msg.GetKey() != 12 -> %v`, msg.GetKey())
	}

	if msg.GetMeta() == nil {
		t.Errorf(`msg.GetMeta() == nil`)
	}

	if msg.GetStringValue()[0] != "testC" {
		t.Errorf(`msg.GetStringValue() != "testA"`)
	}

	if msg.String() != `Meta:<Kind:"MessageC" Key:"12" > Key:12 StringValue:"testC" ` {
		t.Errorf(`msg.String() != Meta:<Kind:"MessageC" Key:"12" > Key:12 StringValue:"testC" `)
	}

	_, desc := descriptor.ForMessage(msg)
	if *desc.Name != "MessageC_" {
		t.Errorf(`*desc.Name != "MessageC_"`)
	}

	data, err := proto.Marshal(msg)
	if err != nil {
		t.Errorf("%v", err)
	}

	base, err := messageCFactory(client, data)
	if err != nil {
		t.Errorf("%v", err)
	}

	c2, ok := base.(*MessageC)
	if !ok {
		t.Errorf("b2, ok := base.(*MessageC_)")
	}

	if c2.GetKey() != 12 {
		t.Errorf(`c2.Key().GetValue() == 12`)
	}

	r := new(reactC)
	rl := make([]gosdk.BaseReactor, 1)
	rl[0] = r

	c2.TriggerEvent(nil, delphi.ObjectOperation_SetOp, rl)
	c2.TriggerEvent(c2, delphi.ObjectOperation_SetOp, rl)
	c2.TriggerEvent(c2, delphi.ObjectOperation_DeleteOp, rl)

	msg.Reset()
	if msg.String() != "" {
		t.Errorf(`msg.String() != ""`)
	}

	if GetMessageC(client, 1) != nil {
		t.Errorf(`GetMessageC("", "") != nil`)
	}
}
