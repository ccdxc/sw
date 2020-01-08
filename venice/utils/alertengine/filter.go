package alertengine

import (
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// Match matches the given set of requirements against the object and
// returns true or false accordingly. Also, Observed values for each requirement is
// returned as set of monitoring.MatchedRequirement.
func Match(reqs []*fields.Requirement, obj runtime.Object) (bool, []*monitoring.MatchedRequirement) {
	if len(reqs) == 0 {
		return false, nil
	}

	fs := &fields.Selector{
		Requirements: reqs,
	}

	schemaType := runtime.GetDefaultScheme().Kind2SchemaType(obj.GetObjectKind())

	// ensures all the fields are valid and their values are intact with the field type
	if err := fs.ValidateRequirements(schemaType, false); err != nil {
		log.Errorf("failed to validate the requirements, err: %v", err)
		return false, nil
	}

	var reqsWithObservedVal []*monitoring.MatchedRequirement
	for _, req := range reqs {
		match, observedVal := req.MatchesObjWithObservedValue(obj)
		if !match {
			return false, nil
		}

		reqsWithObservedVal = append(reqsWithObservedVal, &monitoring.MatchedRequirement{
			Requirement:   req,
			ObservedValue: observedVal,
		})
	}

	return true, reqsWithObservedVal
}
