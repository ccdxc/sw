// Copyright 2016-2017 VMware, Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package simulator

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"reflect"
	"testing"
	"time"

	"github.com/pensando/vic/pkg/vsphere/simulator/esx"
	"github.com/pensando/vic/pkg/vsphere/simulator/vc"
	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25"
	"github.com/vmware/govmomi/vim25/methods"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/soap"
	"github.com/vmware/govmomi/vim25/types"
)

const (
	testTagID = "urn:vmomi:InventoryServiceTag:be1321e5-9539-4914-9abf-d39126122f42:GLOBAL"
)

func TestUnmarshal(t *testing.T) {
	requests := []struct {
		body interface{}
		data string
	}{
		{
			&types.RetrieveServiceContent{
				This: types.ManagedObjectReference{
					Type: "ServiceInstance", Value: "ServiceInstance",
				},
			},
			`<?xml version="1.0" encoding="UTF-8"?>
                         <Envelope xmlns="http://schemas.xmlsoap.org/soap/envelope/">
                           <Body>
                             <RetrieveServiceContent xmlns="urn:vim25">
                               <_this type="ServiceInstance">ServiceInstance</_this>
                             </RetrieveServiceContent>
                           </Body>
                         </Envelope>`,
		},
		{
			&types.Login{
				This: types.ManagedObjectReference{
					Type:  "SessionManager",
					Value: "SessionManager",
				},
				UserName: "root",
				Password: "secret",
			},
			`<?xml version="1.0" encoding="UTF-8"?>
                         <Envelope xmlns="http://schemas.xmlsoap.org/soap/envelope/">
                           <Body>
                             <Login xmlns="urn:vim25">
                               <_this type="SessionManager">SessionManager</_this>
                               <userName>root</userName>
                               <password>secret</password>
                             </Login>
                           </Body>
                         </Envelope>`,
		},
		{
			&types.RetrieveProperties{
				This: types.ManagedObjectReference{Type: "PropertyCollector", Value: "ha-property-collector"},
				SpecSet: []types.PropertyFilterSpec{
					{
						DynamicData: types.DynamicData{},
						PropSet: []types.PropertySpec{
							{
								DynamicData: types.DynamicData{},
								Type:        "ManagedEntity",
								All:         (*bool)(nil),
								PathSet:     []string{"name", "parent"},
							},
						},
						ObjectSet: []types.ObjectSpec{
							{
								DynamicData: types.DynamicData{},
								Obj:         types.ManagedObjectReference{Type: "Folder", Value: "ha-folder-root"},
								Skip:        types.NewBool(false),
								SelectSet: []types.BaseSelectionSpec{ // test decode of interface
									&types.TraversalSpec{
										SelectionSpec: types.SelectionSpec{
											DynamicData: types.DynamicData{},
											Name:        "traverseParent",
										},
										Type: "ManagedEntity",
										Path: "parent",
										Skip: types.NewBool(false),
										SelectSet: []types.BaseSelectionSpec{
											&types.SelectionSpec{
												DynamicData: types.DynamicData{},
												Name:        "traverseParent",
											},
										},
									},
								},
							},
						},
						ReportMissingObjectsInResults: (*bool)(nil),
					},
				}},
			`<?xml version="1.0" encoding="UTF-8"?>
                         <Envelope xmlns="http://schemas.xmlsoap.org/soap/envelope/">
                          <Body>
                           <RetrieveProperties xmlns="urn:vim25">
                            <_this type="PropertyCollector">ha-property-collector</_this>
                            <specSet>
                             <propSet>
                              <type>ManagedEntity</type>
                              <pathSet>name</pathSet>
                              <pathSet>parent</pathSet>
                             </propSet>
                             <objectSet>
                              <obj type="Folder">ha-folder-root</obj>
                              <skip>false</skip>
                              <selectSet xmlns:XMLSchema-instance="http://www.w3.org/2001/XMLSchema-instance" XMLSchema-instance:type="TraversalSpec">
                               <name>traverseParent</name>
                               <type>ManagedEntity</type>
                               <path>parent</path>
                               <skip>false</skip>
                               <selectSet XMLSchema-instance:type="SelectionSpec">
                                <name>traverseParent</name>
                               </selectSet>
                              </selectSet>
                             </objectSet>
                            </specSet>
                           </RetrieveProperties>
                          </Body>
                         </Envelope>`,
		},
	}

	for i, req := range requests {
		method, err := UnmarshalBody([]byte(req.data))
		if err != nil {
			t.Errorf("failed to decode %d (%s): %s", i, req, err)
		}
		if !reflect.DeepEqual(method.Body, req.body) {
			t.Errorf("malformed body %d (%#v):", i, method.Body)
		}
	}
}

func TestUnmarshalError(t *testing.T) {
	requests := []string{
		"", // io.EOF
		`<?xml version="1.0" encoding="UTF-8"?>
                 <Envelope xmlns="http://schemas.xmlsoap.org/soap/envelope/">
                   <Body>
                   </MissingEndTag
                 </Envelope>`,
		`<?xml version="1.0" encoding="UTF-8"?>
                 <Envelope xmlns="http://schemas.xmlsoap.org/soap/envelope/">
                   <Body>
                     <UnknownType xmlns="urn:vim25">
                       <_this type="ServiceInstance">ServiceInstance</_this>
                     </UnknownType>
                   </Body>
                 </Envelope>`,
		`<?xml version="1.0" encoding="UTF-8"?>
                 <Envelope xmlns="http://schemas.xmlsoap.org/soap/envelope/">
                   <Body>
                   <!-- no start tag -->
                   </Body>
                 </Envelope>`,
		`<?xml version="1.0" encoding="UTF-8"?>
                 <Envelope xmlns="http://schemas.xmlsoap.org/soap/envelope/">
                   <Body>
                     <RetrieveServiceContent xmlns="urn:vim25">
                       <_this type="ServiceInstance">ServiceInstance</_this>
                     </RetrieveServiceContent>
                   </Body>
                 </Envelope>`,
	}

	defer func() {
		typeFunc = types.TypeFunc() // reset
	}()

	ttypes := map[string]reflect.Type{
		// triggers xml.Decoder.DecodeElement error
		"RetrieveServiceContent": reflect.TypeOf(nil),
	}
	typeFunc = func(name string) (reflect.Type, bool) {
		typ, ok := ttypes[name]
		return typ, ok
	}

	for i, data := range requests {
		_, err := UnmarshalBody([]byte(data))
		if err != nil {
			continue
		}
		t.Errorf("expected %d (%s) to return an error", i, data)
	}
}

func TestServeHTTP(t *testing.T) {
	configs := []struct {
		content types.ServiceContent
		folder  mo.Folder
	}{
		{esx.ServiceContent, esx.RootFolder},
		{vc.ServiceContent, vc.RootFolder},
	}

	for _, config := range configs {
		s := New(NewServiceInstance(config.content, config.folder))
		ts := s.NewServer()
		defer ts.Close()

		ctx := context.Background()
		client, err := govmomi.NewClient(ctx, ts.URL, true)
		if err != nil {
			t.Fatal(err)
		}

		err = client.Login(ctx, nil)
		if err == nil {
			t.Fatal("expected invalid login error")
		}

		err = client.Login(ctx, ts.URL.User)
		if err != nil {
			t.Fatal(err)
		}

		// Testing http client + reflect client
		clients := []soap.RoundTripper{client, s.client}
		for _, c := range clients {
			now, err := methods.GetCurrentTime(ctx, c)
			if err != nil {
				t.Fatal(err)
			}

			if now.After(time.Now()) {
				t.Fail()
			}

			// test the fail/Fault path
			_, err = methods.QueryVMotionCompatibility(ctx, c, &types.QueryVMotionCompatibility{})
			if err == nil {
				t.Errorf("expected error")
			}
		}

		err = client.Logout(ctx)
		if err != nil {
			t.Error(err)
		}
	}
}

func TestServeAbout(t *testing.T) {
	ctx := context.Background()

	m := VPX()
	m.App = 1
	m.Pod = 1

	defer m.Remove()

	err := m.Create()
	if err != nil {
		t.Fatal(err)
	}

	s := m.Service.NewServer()
	defer s.Close()

	c, err := govmomi.NewClient(ctx, s.URL, true)
	if err != nil {
		t.Fatal(err)
	}

	u := *s.URL
	u.Path += "/vimServiceVersions.xml"
	r, err := c.Get(u.String())
	if err != nil {
		t.Fatal(err)
	}
	_ = r.Body.Close()

	u.Path = "/about"
	r, err = c.Get(u.String())
	if err != nil {
		t.Fatal(err)
	}
	_ = r.Body.Close()
}

func TestServeHTTPS(t *testing.T) {
	s := New(NewServiceInstance(esx.ServiceContent, esx.RootFolder))
	s.TLS = new(tls.Config)
	ts := s.NewServer()
	defer ts.Close()

	ts.Config.ErrorLog = log.New(ioutil.Discard, "", 0) // silence benign "TLS handshake error" log messages

	ctx := context.Background()

	// insecure=true OK
	client, err := govmomi.NewClient(ctx, ts.URL, true)
	if err != nil {
		t.Fatal(err)
	}

	err = client.Login(ctx, ts.URL.User)
	if err != nil {
		t.Fatal(err)
	}

	// insecure=false should FAIL
	_, err = govmomi.NewClient(ctx, ts.URL, false)
	if err == nil {
		t.Fatal("expected error")
	}

	uerr, ok := err.(*url.Error)
	if !ok {
		t.Fatalf("err type=%T", err)
	}

	_, ok = uerr.Err.(x509.UnknownAuthorityError)
	if !ok {
		t.Fatalf("err type=%T", uerr.Err)
	}

	sinfo := ts.CertificateInfo()

	// Test thumbprint validation
	sc := soap.NewClient(ts.URL, false)
	// Add host with thumbprint mismatch should fail
	sc.SetThumbprint(ts.URL.Host, "nope")
	_, err = vim25.NewClient(ctx, sc)
	if err == nil {
		t.Error("expected error")
	}
	// Add host with thumbprint match should pass
	sc.SetThumbprint(ts.URL.Host, sinfo.ThumbprintSHA1)
	_, err = vim25.NewClient(ctx, sc)
	if err != nil {
		t.Fatal(err)
	}

	var pinfo object.HostCertificateInfo
	err = pinfo.FromURL(ts.URL, nil)
	if err != nil {
		t.Fatal(err)
	}
	if pinfo.ThumbprintSHA1 != sinfo.ThumbprintSHA1 {
		t.Error("thumbprint mismatch")
	}

	// Test custom RootCAs list
	sc = soap.NewClient(ts.URL, false)
	caFile, err := ts.CertificateFile()
	if err != nil {
		t.Fatal(err)
	}
	if err = sc.SetRootCAs(caFile); err != nil {
		t.Fatal(err)
	}

	_, err = vim25.NewClient(ctx, sc)
	if err != nil {
		t.Fatal(err)
	}
}

type errorMarshal struct {
	mo.ServiceInstance
}

func (*errorMarshal) Fault() *soap.Fault {
	return nil
}

func (*errorMarshal) MarshalText() ([]byte, error) {
	return nil, errors.New("time has stopped")
}

func (h *errorMarshal) CurrentTime(types.AnyType) soap.HasFault {
	return h
}

type errorNoSuchMethod struct {
	mo.ServiceInstance
}

func TestServeHTTPErrors(t *testing.T) {
	s := New(NewServiceInstance(esx.ServiceContent, esx.RootFolder))

	ts := s.NewServer()
	defer ts.Close()

	ctx := context.Background()
	client, err := govmomi.NewClient(ctx, ts.URL, true)
	if err != nil {
		t.Fatal(err)
	}

	// test response to unimplemented method
	req := &types.QueryMemoryOverhead{This: esx.HostSystem.Reference()}
	_, err = methods.QueryMemoryOverhead(ctx, client.Client, req)
	if _, ok := soap.ToSoapFault(err).VimFault().(types.MethodNotFound); !ok {
		t.Error("expected MethodNotFound fault")
	}

	// unregister type, covering the ServeHTTP UnmarshalBody error path
	typeFunc = func(name string) (reflect.Type, bool) {
		return nil, false
	}

	_, err = methods.GetCurrentTime(ctx, client)
	if err == nil {
		t.Error("expected error")
	}

	typeFunc = types.TypeFunc() // reset

	// cover the does not implement method error path
	Map.objects[serviceInstance] = &errorNoSuchMethod{}
	_, err = methods.GetCurrentTime(ctx, client)
	if err == nil {
		t.Error("expected error")
	}

	// cover the xml encode error path
	Map.objects[serviceInstance] = &errorMarshal{}
	_, err = methods.GetCurrentTime(ctx, client)
	if err == nil {
		t.Error("expected error")
	}

	// cover the no such object path
	Map.Remove(serviceInstance)
	_, err = methods.GetCurrentTime(ctx, client)
	if err == nil {
		t.Error("expected error")
	}

	// verify we properly marshal the fault
	fault := soap.ToSoapFault(err).VimFault()
	f, ok := fault.(types.ManagedObjectNotFound)
	if !ok {
		t.Fatalf("fault=%#v", fault)
	}
	if f.Obj != serviceInstance.Reference() {
		t.Errorf("obj=%#v", f.Obj)
	}

	// cover the method not supported path
	res, err := http.Get(ts.URL.String())
	if err != nil {
		log.Fatal(err)
	}

	if res.StatusCode != http.StatusMethodNotAllowed {
		t.Errorf("expected status %d, got %s", http.StatusMethodNotAllowed, res.Status)
	}

	// cover the ioutil.ReadAll error path
	s.readAll = func(io.Reader) ([]byte, error) {
		return nil, io.ErrShortBuffer
	}
	res, err = http.Post(ts.URL.String(), "none", nil)
	if err != nil {
		log.Fatal(err)
	}

	if res.StatusCode != http.StatusBadRequest {
		t.Errorf("expected status %d, got %s", http.StatusBadRequest, res.Status)
	}
}

func Test_reqParams(t *testing.T) {
	path := fmt.Sprintf("/rest/com/vmware/cis/tagging/tag/id:%s", testTagID)
	parms := reqParams(path, "/", []string{":"})
	if parms["id"] != testTagID {
		t.Errorf("Expected %s, got %s", testTagID,
			parms["id"])
	}

	if len(parms) != 1 {
		t.Errorf("Expected %d params, got %d", 1, len(parms))
	}

	path = fmt.Sprintf("/rest/com/vmware/cis/tagging/tag-association/id:%s?~action=list-attached-objects", testTagID)
	parms = reqParams(path, "/", []string{":", "?", "="})
	if len(parms) != 2 {
		t.Errorf("Expected %d params, got %d", 2, len(parms))
	}
	if parms["id"] != testTagID {
		t.Errorf("Expected %s, got %s", testTagID,
			parms["id"])
	}
	if parms["~action"] != "list-attached-objects" {
		t.Errorf("Expected list-attached-objects, got %s", parms["~action"])
	}

	path = fmt.Sprintf("/rest/com/vmware/cis/tagging/tag-association/id:%s?~action=attach", testTagID)
	parms = reqParams(path, "/", []string{":", "?", "="})
	if len(parms) != 2 {
		t.Errorf("Expected %d params, got %d", 2, len(parms))
	}
	if parms["id"] != testTagID {
		t.Errorf("Expected %s, got %s", testTagID,
			parms["id"])
	}
	if parms["~action"] != "attach" {
		t.Errorf("Expected attach, got %s", parms["~action"])
	}
	path = "/rest/com/vmware/cis/tagging/tag-association?~action=list-attached-objects-on-tags"
	parms = reqParams(path, "?", []string{"="})
	if len(parms) != 1 {
		t.Errorf("Expected %d params, got %d", 1, len(parms))
	}
	if parms["~action"] != "list-attached-objects-on-tags" {
		t.Errorf("Expected list-attached-objects-on-tags, got %s", parms["~action"])
	}
}

func TestTags(t *testing.T) {
	s := New(NewServiceInstance(esx.ServiceContent, esx.RootFolder))

	ts := s.NewServer()
	defer ts.Close()

	tc := NewTagClient(ts.URL)

	// CreateSession
	err := tc.CreateSession(context.Background())
	if err != nil {
		t.Fatal(err)
	}

	// CreateTag
	err = tc.CreateTag("tagA")
	if err != nil {
		t.Fatal(err)
	}

	// GetTagByName
	id, err := tc.GetTagByName("tagA")
	if err != nil {
		t.Fatal(err)
	}

	if id == "" {
		t.Fatalf("GetTagByName failed")
	}

	// UpdateTag
	err = tc.UpdateTag(id, "tagB")
	if err != nil {
		t.Fatal(err)
	}

	tn, _, err := tc.GetTag(id)
	if err != nil {
		t.Fatal(err)
	}

	if tn != "tagB" {
		t.Errorf("Update failed")
	}

	err = tc.CreateTag("tagABC")
	if err != nil {
		t.Fatal(err)
	}

	tags, err := tc.ListTags()
	if err != nil {
		t.Fatal(err)
	}

	if len(tags) != 2 {
		t.Errorf("Expected two tags, got %d", len(tags))
	}

	err = tc.DeleteTag("tagB")
	if err != nil {
		t.Fatal(err)
	}

	// Attach
	err = tc.Attach("tagABC", "vm-101")
	if err != nil {
		t.Fatal(err)
	}

	idABC, err := tc.GetTagByName("tagABC")
	if err != nil {
		t.Fatal(err)
	}

	// ListVMs
	vms, err := tc.ListVMs(idABC)
	if err != nil {
		t.Fatal(err)
	}

	if len(vms) != 1 || vms[0] != "vm-101" {
		t.Fatalf("ListVMs failed %+v", vms)
	}
	err = tc.Attach("tagABC", "vm-102")
	if err != nil {
		t.Fatal(err)
	}

	// create another tag
	err = tc.CreateTag("tagXYZ")
	if err != nil {
		t.Fatal(err)
	}
	err = tc.Attach("tagXYZ", "vm-103")
	if err != nil {
		t.Fatal(err)
	}

	idXYZ, err := tc.GetTagByName("tagXYZ")
	if err != nil {
		t.Fatal(err)
	}
	// ListVMsMulti
	tags = []string{idABC, idXYZ}
	to, err := tc.ListVMsMulti(tags)
	if err != nil {
		t.Fatal(err)
	}

	if len(to) != 2 {
		t.Errorf("Expected 2 object sets, got %d",
			len(to))
	}

	// DeleteSession
	err = tc.DeleteSession()
	if err != nil {
		t.Fatal(err)
	}

	// Detach
	err = tc.Detach("tagABC", "vm-102")
	if err == nil {
		t.Fatal("Expected error, got none")
	}

	// Create session
	err = tc.CreateSession(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	// Detach
	err = tc.Detach("tagABC", "vm-102")
	if err != nil {
		t.Fatal(err)
	}
	err = tc.DeleteTag("tagXYZ")
	if err != nil {
		t.Fatal(err)
	}
	// ListVMsMulti
	tags = []string{idABC, idXYZ}
	to, err = tc.ListVMsMulti(tags)
	if err != nil {
		t.Fatal(err)
	}

	found := false
	for _, obj := range to {
		if len(obj.ObjIDs) != 0 {
			if obj.ObjIDs[0].ID == "vm-101" {
				found = true
			}
			break
		}
	}

	if !found {
		t.Errorf("Error listing %+v", to)
	}
}
