// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package cluster is a auto generated package.
Input file: credentials.proto
*/
package cluster

import (
	"context"
	"errors"
	fmt "fmt"

	listerwatcher "github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"

	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/transformers/storage"
)

// Dummy definitions to suppress nonused warnings
var _ kvstore.Interface
var _ log.Logger
var _ listerwatcher.WatcherClient

var storageTransformersMapCredentials = make(map[string][]func(ctx context.Context, i interface{}, toStorage bool) error)
var eraseSecretsMapCredentials = make(map[string]func(i interface{}))

// MakeKey generates a KV store key for the object
func (m *Credentials) MakeKey(prefix string) string {
	return fmt.Sprint(globals.ConfigRootPrefix, "/", prefix, "/", "credentials/", m.Name)
}

func (m *Credentials) MakeURI(cat, ver, prefix string) string {
	in := m
	return fmt.Sprint("/", cat, "/", prefix, "/", ver, "/credentials/", in.Name)
}

// Clone clones the object into into or creates one of into is nil
func (m *Credentials) Clone(into interface{}) (interface{}, error) {
	var out *Credentials
	var ok bool
	if into == nil {
		out = &Credentials{}
	} else {
		out, ok = into.(*Credentials)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *(ref.DeepCopy(m).(*Credentials))
	return out, nil
}

// Default sets up the defaults for the object
func (m *Credentials) Defaults(ver string) bool {
	var ret bool
	m.Kind = "Credentials"
	ret = m.Tenant != "" || m.Namespace != ""
	if ret {
		m.Tenant, m.Namespace = "", ""
	}
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *CredentialsSpec) Clone(into interface{}) (interface{}, error) {
	var out *CredentialsSpec
	var ok bool
	if into == nil {
		out = &CredentialsSpec{}
	} else {
		out, ok = into.(*CredentialsSpec)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *(ref.DeepCopy(m).(*CredentialsSpec))
	return out, nil
}

// Default sets up the defaults for the object
func (m *CredentialsSpec) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *CredentialsStatus) Clone(into interface{}) (interface{}, error) {
	var out *CredentialsStatus
	var ok bool
	if into == nil {
		out = &CredentialsStatus{}
	} else {
		out, ok = into.(*CredentialsStatus)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *(ref.DeepCopy(m).(*CredentialsStatus))
	return out, nil
}

// Default sets up the defaults for the object
func (m *CredentialsStatus) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *KeyValue) Clone(into interface{}) (interface{}, error) {
	var out *KeyValue
	var ok bool
	if into == nil {
		out = &KeyValue{}
	} else {
		out, ok = into.(*KeyValue)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *(ref.DeepCopy(m).(*KeyValue))
	return out, nil
}

// Default sets up the defaults for the object
func (m *KeyValue) Defaults(ver string) bool {
	return false
}

// Validators and Requirements

func (m *Credentials) References(tenant string, path string, resp map[string]apiintf.ReferenceObj) {

}

func (m *Credentials) Validate(ver, path string, ignoreStatus bool, ignoreSpec bool) []error {
	var ret []error

	if m.Tenant != "" {
		ret = append(ret, errors.New("Tenant not allowed for Credentials"))
	}
	if m.Namespace != "" {
		ret = append(ret, errors.New("Namespace not allowed for Credentials"))
	}

	{
		dlmtr := "."
		if path == "" {
			dlmtr = ""
		}
		npath := path + dlmtr + "ObjectMeta"
		if errs := m.ObjectMeta.Validate(ver, npath, ignoreStatus, ignoreSpec); errs != nil {
			ret = append(ret, errs...)
		}
	}
	return ret
}

func (m *Credentials) Normalize() {

	m.ObjectMeta.Normalize()

}

func (m *CredentialsSpec) References(tenant string, path string, resp map[string]apiintf.ReferenceObj) {

}

func (m *CredentialsSpec) Validate(ver, path string, ignoreStatus bool, ignoreSpec bool) []error {
	var ret []error
	return ret
}

func (m *CredentialsSpec) Normalize() {

}

func (m *CredentialsStatus) References(tenant string, path string, resp map[string]apiintf.ReferenceObj) {

}

func (m *CredentialsStatus) Validate(ver, path string, ignoreStatus bool, ignoreSpec bool) []error {
	var ret []error
	return ret
}

func (m *CredentialsStatus) Normalize() {

}

func (m *KeyValue) References(tenant string, path string, resp map[string]apiintf.ReferenceObj) {

}

func (m *KeyValue) Validate(ver, path string, ignoreStatus bool, ignoreSpec bool) []error {
	var ret []error
	return ret
}

func (m *KeyValue) Normalize() {

}

// Transformers

func (m *Credentials) ApplyStorageTransformer(ctx context.Context, toStorage bool) error {
	if err := m.Spec.ApplyStorageTransformer(ctx, toStorage); err != nil {
		return err
	}
	return nil
}

func (m *Credentials) EraseSecrets() {
	m.Spec.EraseSecrets()

	return
}

type storageCredentialsTransformer struct{}

var StorageCredentialsTransformer storageCredentialsTransformer

func (st *storageCredentialsTransformer) TransformFromStorage(ctx context.Context, i interface{}) (interface{}, error) {
	r := i.(Credentials)
	err := r.ApplyStorageTransformer(ctx, false)
	if err != nil {
		return nil, err
	}
	return r, nil
}

func (st *storageCredentialsTransformer) TransformToStorage(ctx context.Context, i interface{}) (interface{}, error) {
	r := i.(Credentials)
	err := r.ApplyStorageTransformer(ctx, true)
	if err != nil {
		return nil, err
	}
	return r, nil
}

func (m *CredentialsSpec) ApplyStorageTransformer(ctx context.Context, toStorage bool) error {
	for i, v := range m.KeyValuePairs {
		c := v
		if err := c.ApplyStorageTransformer(ctx, toStorage); err != nil {
			return err
		}
		m.KeyValuePairs[i] = c
	}
	return nil
}

func (m *CredentialsSpec) EraseSecrets() {
	for _, v := range m.KeyValuePairs {
		v.EraseSecrets()
	}
	return
}

func (m *KeyValue) ApplyStorageTransformer(ctx context.Context, toStorage bool) error {
	if vs, ok := storageTransformersMapCredentials["KeyValue"]; ok {
		for _, v := range vs {
			if err := v(ctx, m, toStorage); err != nil {
				return err
			}
		}
	}
	return nil
}

func (m *KeyValue) EraseSecrets() {
	if v, ok := eraseSecretsMapCredentials["KeyValue"]; ok {
		v(m)
	}
	return
}

func init() {
	scheme := runtime.GetDefaultScheme()
	scheme.AddKnownTypes(
		&Credentials{},
	)

	{
		KeyValueValueTx, err := storage.NewSecretValueTransformer()
		if err != nil {
			log.Fatalf("Error instantiating SecretStorageTransformer: %v", err)
		}
		storageTransformersMapCredentials["KeyValue"] = append(storageTransformersMapCredentials["KeyValue"],
			func(ctx context.Context, i interface{}, toStorage bool) error {
				var data []byte
				var err error
				m := i.(*KeyValue)

				if toStorage {
					data, err = KeyValueValueTx.TransformToStorage(ctx, []byte(m.Value))
				} else {
					data, err = KeyValueValueTx.TransformFromStorage(ctx, []byte(m.Value))
				}
				m.Value = []byte(data)

				return err
			})

		eraseSecretsMapCredentials["KeyValue"] = func(i interface{}) {
			m := i.(*KeyValue)

			var data []byte
			m.Value = []byte(data)

			return
		}

	}

}
