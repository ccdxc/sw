package impl

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/log"
)

func TestWorkloadObject(t *testing.T) {
	wl := &wlHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Workload-Hooks-Test")),
	}

	// Testcases for various host configs
	hostTestcases := []struct {
		obj workload.Workload
		err []error
	}{
		// invalid mac addr #1
		{
			workload.Workload{
				ObjectMeta: api.ObjectMeta{
					Name: "coke-vm-1",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Workload",
					APIVersion: "v1",
				},
				Spec: workload.WorkloadSpec{
					Interfaces: map[string]workload.WorkloadIntfSpec{
						"01.02.03.04.05.06": workload.WorkloadIntfSpec{},
					},
				},
			},
			[]error{
				wl.errInvalidMacConfig("01.02.03.04.05.06"),
			},
		},
		// invalid mac addr #2
		{
			workload.Workload{
				ObjectMeta: api.ObjectMeta{
					Name: "coke-vm-2",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Workload",
					APIVersion: "v1",
				},
				Spec: workload.WorkloadSpec{
					Interfaces: map[string]workload.WorkloadIntfSpec{
						"hello-world": workload.WorkloadIntfSpec{},
					},
				},
			},
			[]error{
				wl.errInvalidMacConfig("hello-world"),
			},
		},
		// valid workload object
		{
			workload.Workload{
				ObjectMeta: api.ObjectMeta{
					Name: "coke-vm-3",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Workload",
					APIVersion: "v1",
				},
				Spec: workload.WorkloadSpec{
					Interfaces: map[string]workload.WorkloadIntfSpec{
						"0202.0404.0606": workload.WorkloadIntfSpec{
							// MicroSegVlan will be allocated by backend (NPM)
							ExternalVlan: 1001,
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
			err := wl.validateWorkloadConfig(tc.obj, "", true)
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
