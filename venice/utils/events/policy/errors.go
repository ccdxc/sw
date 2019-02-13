package policy

import (
	"errors"
)

var (
	// ErrFailedToCreatePolicy indicates a failure in the policy creation
	ErrFailedToCreatePolicy = errors.New("failed to create policy")

	// ErrPolicyExistsAlready indicates the policy exists already in the store
	ErrPolicyExistsAlready = errors.New("policy exists already")

	// ErrPolicyDoesNotExists indicates the policy does not exists in the store
	ErrPolicyDoesNotExists = errors.New("policy does not exists")

	// ErrFailedToGetPolicy indicates the failure in retrieving policy from the store
	ErrFailedToGetPolicy = errors.New("failed to get policy")

	// ErrFailedToUpdatePolicy indicates the failure in updating existing policy
	ErrFailedToUpdatePolicy = errors.New("failed to update policy")

	// ErrFailedToDeletePolicy indicates the failure in policy deletion
	ErrFailedToDeletePolicy = errors.New("failed to delete policy")

	// ErrFailedToListPolicies indicates the failure in retrieving policies from the store
	ErrFailedToListPolicies = errors.New("failed to list policies")
)
