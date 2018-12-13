package dependencies

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	benchmarkObjCount = 100000
)

func BenchmarkDepSolver_AddRemove(b *testing.B) {
	b.StopTimer()
	s := NewDepSolver()
	parent := netproto.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "tenant"},
		ObjectMeta: api.ObjectMeta{Name: "testTenant"},
	}
	b.StartTimer()
	for i := 0; i < benchmarkObjCount; i++ {
		c := netproto.Namespace{
			TypeMeta: api.TypeMeta{Kind: "namespace"},
			ObjectMeta: api.ObjectMeta{
				Tenant: "testTenant",
				Name:   fmt.Sprintf("testNamespace-%d", i),
			},
		}
		err := s.Add(&parent, &c)
		AssertOk(b, err, "failed to add dependency")
		err = s.Remove(&parent, &c)
		AssertOk(b, err, "failed to remove dependency")

	}
}

func BenchmarkDepSolver_Solve(b *testing.B) {
	b.StopTimer()
	s := NewDepSolver()
	parent := netproto.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "tenant"},
		ObjectMeta: api.ObjectMeta{Name: "testTenant"},
	}
	for i := 0; i < benchmarkObjCount; i++ {
		c := netproto.Namespace{
			TypeMeta: api.TypeMeta{Kind: "namespace"},
			ObjectMeta: api.ObjectMeta{
				Tenant: "testTenant",
				Name:   fmt.Sprintf("testNamespace-%d", i),
			},
		}
		err := s.Add(&parent, &c)
		AssertOk(b, err, "failed to add dependency")
	}
	b.StartTimer()
	err := s.Solve(&parent)
	Assert(b, err != nil, "solver should report error cannot delete")
}
