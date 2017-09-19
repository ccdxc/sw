package dh

import (
	"crypto/rand"
	"fmt"
	"math/big"
)

// Key is a representation of a Diffie Hellman key and its state.
type Key struct {
	id     GroupID
	x      *big.Int // my priv key
	gx     *big.Int // (g ^ x) mod p
	gy     *big.Int // (g ^ y) mod p (sent by peer)
	secret *big.Int // (g ^ xy) mod p (shared secret negotiated)
}

const (
	maxRandAttempts = 10
)

// NewKey returns a Key for a given DH group with random number x and local
// exponentiated value (g ^ x) mod p.
func NewKey(id GroupID) (*Key, error) {
	group := GetGroup(id)

	// x should be in the range 1..p-1
	x, err := rand.Int(rand.Reader, group.prime)
	if err != nil {
		return nil, err
	}

	z := big.NewInt(0)
	ii := 0
	for z.Cmp(x) == 0 && ii < maxRandAttempts {
		x, err = rand.Int(rand.Reader, group.prime)
		if err != nil {
			return nil, err
		}
		ii++
	}

	if ii == maxRandAttempts {
		return nil, fmt.Errorf("Failed to generate a valid randum number in %d attempts", maxRandAttempts)
	}

	return &Key{
		id: id,
		x:  x,
		gx: new(big.Int).Exp(group.generator, x, group.prime),
	}, nil
}

// LocalExponent returns a key's local exponent (g ^ x) mod p.
func (k *Key) LocalExponent() *big.Int {
	return k.gx
}

// ComputeSecret takes peer's exponentiated value (g ^ y) mod p and returns (g ^ xy) mod p,
// which is the shared secret.
func (k *Key) ComputeSecret(gy *big.Int) (*big.Int, error) {
	if k.id == GroupID(0) || k.x == nil || k.gx == nil || k.gy != nil || k.secret != nil {
		return nil, fmt.Errorf("Invalid DH key for computing secret")
	}

	group := GetGroup(k.id)

	k.gy = gy
	k.secret = new(big.Int).Exp(gy, k.x, group.prime)
	return k.secret, nil
}
