// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package storage

import (
	"context"
)

// ValueTransformer provides methods to transform a value to/from a form suitable for persistent storage.
// For example, a value may need to be encoded, annotated or encrypted.
// Transformations must be reversible, so that they can be undone after reading the value back.
type ValueTransformer interface {
	// TransformToStorage is invoked right before writing the value to persistent storage
	// The original data is passed in is unchanged.
	TransformToStorage(ctx context.Context, data []byte) ([]byte, error)
	// TransformFromStorage is invoked right after reading the value from persistent storage
	// The original data is passed in is unchanged.
	TransformFromStorage(ctx context.Context, data []byte) ([]byte, error)
}
