package impl

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
)

func TestHostObject(t *testing.T) {
	cl := &clHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Host-Hooks-Test")),
	}

	// Testcases for various host configs
	hostTestcases := []struct {
		obj cluster.Host
		err []error
	}{
		// invalid host name
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: ".naples1-host.local",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
			},
			[]error{
				cl.errInvalidHostConfig(".naples1-host.local"),
			},
		},
		// invalid mac addr #1
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "naples2-host.local",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					Interfaces: map[string]cluster.HostIntfSpec{
						"hello-world": cluster.HostIntfSpec{},
					},
				},
			},
			[]error{
				cl.errInvalidMacConfig("hello-world"),
			},
		},
		// invalid mac addr #2
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "10.5.5.5",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					Interfaces: map[string]cluster.HostIntfSpec{
						"01.02.03.04.05.06": cluster.HostIntfSpec{
							MacAddrs: []string{"0102.0304.05.06"},
						},
					},
				},
			},
			[]error{
				cl.errInvalidMacConfig("01.02.03.04.05.06"),
				cl.errInvalidMacConfig("0102.0304.05.06"),
			},
		},
		// valid host object #1
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "naples3-host.local",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					Interfaces: map[string]cluster.HostIntfSpec{
						"0102.0304.0506": cluster.HostIntfSpec{
							MacAddrs: []string{"0102.0304.0506"},
						},
					},
				},
			},
			[]error{},
		},
		// valid host object #2
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "20.5.5.5",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					Interfaces: map[string]cluster.HostIntfSpec{
						"01-02-03-04-05-06": cluster.HostIntfSpec{
							MacAddrs: []string{"0102.0304.0506"},
						},
					},
				},
			},
			[]error{},
		},
	}

	// Execute the host config testcases
	for _, tc := range hostTestcases {
		t.Run(tc.obj.Name, func(t *testing.T) {
			err := cl.validateHostConfig(tc.obj, "", true)
			if len(err) != len(tc.err) {
				t.Errorf("Expected errors: [%d] actual errors: [%+v]", len(tc.err), len(err))
			}
			for i := 0; i < len(err); i++ {
				if tc.err[i].Error() != err[i].Error() {
					t.Errorf("[%s] error[%d] - expected: [%+v] actual: [%+v]", tc.obj.Name, i, tc.err[i], err[i])
				}
			}
		})
	}
}
