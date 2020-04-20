// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package fields_test

import (
	"context"
	"strings"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/client"
	bs "github.com/pensando/sw/api/generated/bookstore"
	_ "github.com/pensando/sw/api/generated/bookstore/grpc/server"
	. "github.com/pensando/sw/api/hooks/apiserver/fields"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

func TestHasImmutableFields(t *testing.T) {
	schema := runtime.GetDefaultScheme()

	// negative testcases -- should jut return default (false)
	Assert(t, HasImmutableFields("bookstore.Customer", nil) == false, "HasImmutableFields returned true for invalid TypeMap")
	Assert(t, HasImmutableFields("foo", schema.Types) == false, "HasImmutableFields returned true for invalid kind")
	Assert(t, HasImmutableFields("", schema.Types) == false, "HasImmutableFields returned true for empty kind")

	// positive testcases
	Assert(t, HasImmutableFields("bookstore.Customer", schema.Types) == true, "HasImmutableFields returned false for kind with immutable fields %+v", schema.Types)
	Assert(t, HasImmutableFields("bookstore.Publisher", schema.Types) == false, "HasImmutableFields returned true for kind without immutable fields")
}

func TestCheckImmutableFieldChanges(t *testing.T) {
	// CheckImmutableFieldChanges should not panic with invalid params
	_, result, err := CheckImmutableFieldChanges(context.TODO(), nil, nil, "", apiintf.UpdateOper, false, nil)
	Assert(t, result == false && err != nil, "CheckImmutableFieldChanges did not return error with invalid parameters")
	_, result, err = CheckImmutableFieldChanges(context.TODO(), nil, nil, "key", apiintf.UpdateOper, false, "")
	Assert(t, result == false && err != nil, "CheckImmutableFieldChanges did not return error with invalid parameters")

	// if oper != Update, CheckImmutableFieldChanges should return true immediately
	_, result, err = CheckImmutableFieldChanges(context.TODO(), nil, nil, "", apiintf.CreateOper, false, nil)
	Assert(t, result == true && err == nil, "CheckImmutableFieldChanges did not return immediately with oper != update")

	// checkImmutableFields with empty/invalid kind
	ok, err := CheckImmutableFields("", nil, nil)
	Assert(t, !ok && err != nil, "checkImmutableFields succeeded with empty kind")
	ok, err = CheckImmutableFields("foo", nil, nil)
	Assert(t, !ok && err != nil, "checkImmutableFields succeeded with invalid kind")

	type testCase struct {
		updObj bs.Customer
		refObj bs.Customer
		err    string
	}

	testCases := []testCase{
		{ // identical object
			bs.Customer{},
			bs.Customer{},
			"",
		},
		{ // immutable field change, non-empty to empty
			bs.Customer{},
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "id",
				},
			},
			"Id",
		},
		{ // immutable field change, empty to non-empty
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "id",
				},
			},
			bs.Customer{},
			"Id",
		},
		{ // immutable field change, non-empty to non-empty
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "id1",
				},
			},
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "id2",
				},
			},
			"Id",
		},
		{ // mutable and immutable field change
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id:      "id1",
					Address: "addr1",
				},
			},
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id:      "id2",
					Address: "addr2",
				},
			},
			"Id",
		},
		{ // nested, repeated immutable field change, non-empty to empty
			bs.Customer{},
			bs.Customer{
				Spec: bs.CustomerSpec{
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777, 7777},
					},
				},
			},
			"LuckyNumbers",
		},
		{ // nested, repeated immutable field change, empty to non-empty
			bs.Customer{
				Spec: bs.CustomerSpec{
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777, 7777},
					},
				},
			},
			bs.Customer{},
			"LuckyNumbers",
		},
		{ // nested, repeated immutable field change, non-empty to non-empty
			bs.Customer{
				Spec: bs.CustomerSpec{
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
				},
			},
			bs.Customer{
				Spec: bs.CustomerSpec{
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777, 7777},
					},
				},
			},
			"LuckyNumbers",
		},
		{ // identical spec, status change
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "Id",
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
				},
				Status: bs.CustomerStatus{
					AccountStatus: "Active",
				},
			},
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "Id",
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
				},
				Status: bs.CustomerStatus{
					AccountStatus: "Inactive",
				},
			},
			"",
		},
		{ // Immutable field change inside an array, adding an item should be fine
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "Id",
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
					SecurityQuestions: []bs.SecurityQuestions{
						bs.SecurityQuestions{
							Question: bs.SecurityQuestions_FirstPet.String(),
							Answer:   "Go the Gopher",
						},
					},
				},
				Status: bs.CustomerStatus{
					AccountStatus: "Active",
				},
			},
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "Id",
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
					SecurityQuestions: []bs.SecurityQuestions{
						bs.SecurityQuestions{
							Question: bs.SecurityQuestions_FirstPet.String(),
							Answer:   "Go the Gopher1",
						},
						bs.SecurityQuestions{
							Question: bs.SecurityQuestions_ChildhoodFriend.String(),
							Answer:   "Go the Gopher",
						},
					},
				},
				Status: bs.CustomerStatus{
					AccountStatus: "Inactive",
				},
			},
			"",
		},
		{ // Immutable field change inside an array, modifying immutable field should fail
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "Id",
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
					SecurityQuestions: []bs.SecurityQuestions{
						bs.SecurityQuestions{
							Question: bs.SecurityQuestions_FirstPet.String(),
							Answer:   "Go the Gopher",
						},
					},
				},
				Status: bs.CustomerStatus{
					AccountStatus: "Active",
				},
			},
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "Id",
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
					SecurityQuestions: []bs.SecurityQuestions{
						bs.SecurityQuestions{
							Question: bs.SecurityQuestions_ChildhoodFriend.String(),
							Answer:   "Go the Gopher",
						},
					},
				},
				Status: bs.CustomerStatus{
					AccountStatus: "Inactive",
				},
			},
			"Question",
		},
		{ // Immutable field change on a map, adding entry is fine
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "Id",
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
					Cart: map[string]*bs.CartItem{
						"id1": &bs.CartItem{
							ID:       "id1",
							Quantity: 1,
						},
					},
				},
				Status: bs.CustomerStatus{
					AccountStatus: "Active",
				},
			},
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "Id",
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
					Cart: map[string]*bs.CartItem{
						"id1": &bs.CartItem{
							ID:       "id1",
							Quantity: 2,
						},
						// "id2": &bs.CartItem{
						// 	ID:       "id2",
						// 	Quantity: 1,
						// },
					},
				},
				Status: bs.CustomerStatus{
					AccountStatus: "Inactive",
				},
			},
			"",
		},
		{ // Immutable field change on a map, modifying immutable field of an entry should fail
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "Id",
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
					Cart: map[string]*bs.CartItem{
						"id1": &bs.CartItem{
							ID:       "id1",
							Quantity: 1,
						},
					},
				},
				Status: bs.CustomerStatus{
					AccountStatus: "Active",
				},
			},
			bs.Customer{
				Spec: bs.CustomerSpec{
					Id: "Id",
					PasswordRecoveryInfo: bs.CustomerPersonalInfo{
						LuckyNumbers: []uint32{7, 77, 777},
					},
					Cart: map[string]*bs.CartItem{
						"id1": &bs.CartItem{
							ID:       "id2",
							Quantity: 2,
						},
						"id2": &bs.CartItem{
							ID:       "id2",
							Quantity: 1,
						},
					},
				},
				Status: bs.CustomerStatus{
					AccountStatus: "Inactive",
				},
			},
			"ID",
		},
	}

	schema := runtime.GetDefaultScheme()
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(schema)}
	kv, err := store.New(config)
	AssertOk(t, err, "Error instantiating KVStore")

	ctx := context.TODO()
	key := (&bs.Customer{}).MakeKey("")

	for i, tc := range testCases {
		err := kv.Create(ctx, key, &tc.refObj)
		AssertOk(t, err, "Error creating reference object in KVStore")

		tc.updObj.TypeMeta.Kind = "Customer"
		_, ok, err := CheckImmutableFieldChanges(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, tc.updObj)
		if !ok && err == nil {
			t.Fatalf("checkImmutableFields failed but returned no error, test case: %d", i)
		}
		if ok && tc.err != "" {
			t.Fatalf("Expected error %s but checkImmutableFields succeeded, test case: %d", tc.err, i)
		}
		if !ok && tc.err == "" {
			t.Fatalf("Expected success but checkImmutableFields returned error %s, test case: %d", err, i)
		}
		if tc.err != "" && !strings.Contains(err.Error(), tc.err) {
			t.Fatalf("Error returned by checkImmutableFields does not contain expected info. Want: %s, have: %v, test case: %d", tc.err, err, i)
		}

		err = kv.Delete(ctx, key, nil)
		AssertOk(t, err, "Error deleting reference object in KVStore")
	}
}

func TestRegisterImmutableFieldsServiceHooks(t *testing.T) {
	// ApiServer dependencies
	logger := log.GetNewLogger(log.GetDefaultConfig("immutable_fields_test.go"))
	recorder.Override(mock.NewRecorder("apiserver_test", logger))

	// Start ApiServer to trigger invocation of RegisterImmutableFieldsServiceHooks
	srv, addr, err := serviceutils.StartAPIServer("localhost:0", "immutable_fields_test", logger)
	AssertOk(t, err, "Error initializing ApiServer")
	defer srv.Stop()

	// make a call to ApiServer to verify that the precommit hook is registered and active
	apiclient, err := client.NewGrpcUpstream("immutable_fields_test", addr, logger)
	AssertOk(t, err, "Error creating ApiServer client")

	ref := bs.Customer{
		TypeMeta: api.TypeMeta{
			Kind: "Customer",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "test",
		},
		Spec: bs.CustomerSpec{
			Id:      "Alice",
			Address: "addr2",
		},
	}
	_, err = apiclient.BookstoreV1().Customer().Create(context.TODO(), &ref)
	AssertOk(t, err, "Error creating reference object in ApiServer")
	upd := ref
	upd.Spec.Address = "somewhere"
	_, err = apiclient.BookstoreV1().Customer().Update(context.TODO(), &upd)
	AssertOk(t, err, "Error posting valid update to object in ApiServer")
	upd.Spec.Id = "Bob"
	_, err = apiclient.BookstoreV1().Customer().Update(context.TODO(), &upd)
	Assert(t, err != nil, "No error posting invalid update to object in ApiServer")
}
