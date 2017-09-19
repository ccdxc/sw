package kvstore

import (
	"fmt"
)

// Error codes
const (
	ErrCodeKeyExists int = iota + 1
	ErrCodeKeyNotFound
	ErrCodeVersionConflict
	ErrCodeInvalidTTL
	ErrCodeTxnFailed
)

var errCodeToMessage = map[int]string{
	ErrCodeKeyExists:       "Key already exists",
	ErrCodeKeyNotFound:     "Key not found",
	ErrCodeVersionConflict: "Version conflict",
	ErrCodeInvalidTTL:      "Invalid TTL",
	ErrCodeTxnFailed:       "Txn failed",
}

// KVError describes errors associated with kv store.
type KVError struct {
	Code            int
	Message         string
	Key             string
	ResourceVersion int64
	TTL             int64
}

// Error implements the error interface.
func (k *KVError) Error() string {
	return fmt.Sprintf("KVError: %v, key: %v, version: %d", k.Message, k.Key, k.ResourceVersion)
}

func isErrorCode(err error, code int) bool {
	if err == nil {
		return false
	}
	if e, ok := err.(*KVError); ok {
		if e.Code == code {
			return true
		}
	}
	return false
}

// NewKeyExistsError returns a key exists error
func NewKeyExistsError(key string, version int64) *KVError {
	return &KVError{
		Code:            ErrCodeKeyExists,
		Message:         errCodeToMessage[ErrCodeKeyExists],
		Key:             key,
		ResourceVersion: version,
	}
}

// IsKeyExistsError checks if it is key exists error
func IsKeyExistsError(err error) bool {
	return isErrorCode(err, ErrCodeKeyExists)
}

// NewKeyNotFoundError returns a key not found error
func NewKeyNotFoundError(key string, version int64) *KVError {
	return &KVError{
		Code:            ErrCodeKeyNotFound,
		Message:         errCodeToMessage[ErrCodeKeyNotFound],
		Key:             key,
		ResourceVersion: version,
	}
}

// IsKeyNotFoundError checks if the error is 'key not found'
func IsKeyNotFoundError(err error) bool {
	return isErrorCode(err, ErrCodeKeyNotFound)
}

// NewVersionConflictError returns a version conflict error.
func NewVersionConflictError(key string, version int64) *KVError {
	return &KVError{
		Code:            ErrCodeVersionConflict,
		Message:         errCodeToMessage[ErrCodeVersionConflict],
		Key:             key,
		ResourceVersion: version,
	}
}

// IsVersionConflictError checks if the error is a version conflict error.
func IsVersionConflictError(err error) bool {
	return isErrorCode(err, ErrCodeVersionConflict)
}

// NewInvalidTTLError returns an invalid ttl error
func NewInvalidTTLError(ttl int64) *KVError {
	return &KVError{
		Code:    ErrCodeInvalidTTL,
		Message: errCodeToMessage[ErrCodeInvalidTTL],
		Key:     "",
		TTL:     ttl,
	}
}

// IsInvalidTTLError checks if it is invalid TTL error
func IsInvalidTTLError(err error) bool {
	return isErrorCode(err, ErrCodeKeyExists)
}

// NewTxnFailedError returns a txn failed error
func NewTxnFailedError() *KVError {
	return &KVError{
		Code:    ErrCodeTxnFailed,
		Message: errCodeToMessage[ErrCodeTxnFailed],
	}
}

// IsTxnFailedError checks if it is txn failed error
func IsTxnFailedError(err error) bool {
	return isErrorCode(err, ErrCodeTxnFailed)
}
