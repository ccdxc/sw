package password_test

import (
	"fmt"
	"testing"

	. "github.com/pensando/sw/venice/utils/authn/password"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func createPassword(len int) string {
	passwdbytes := make([]byte, len)
	for i := range passwdbytes {
		passwdbytes[i] = 'a'
	}
	return string(passwdbytes)
}

func TestPasswordVerification(t *testing.T) {
	// get password hasher
	hasher := GetPasswordHasher()
	// generate hash
	passwordhash, err := hasher.GetPasswordHash(testPassword)
	AssertOk(t, err, "Error creating password hash")
	// compare hash
	ok, err := hasher.CompareHashAndPassword(passwordhash, testPassword)
	AssertOk(t, err, "Error comparing password hash")
	Assert(t, ok, "Password didn't match")
}

func TestIncorrectPasswordVerification(t *testing.T) {
	// get password hasher
	hasher := GetPasswordHasher()
	// generate hash
	passwordhash, err := hasher.GetPasswordHash(testPassword)
	AssertOk(t, err, "Error creating password hash")
	// compare hash
	ok, err := hasher.CompareHashAndPassword(passwordhash, "wrongpassword")
	Assert(t, (err != nil), "No error returned for incorrect password")
	Assert(t, !ok, "Incorrect password matched")
}

func BenchmarkGetPasswordHash(b *testing.B) {
	// get password hasher
	hasher := GetPasswordHasher()
	// generate hash
	hasher.GetPasswordHash("pensando")
}

// BenchmarkCompareHashAndPassword to find out how much time it takes to verify a hash so that cost could be adjusted accordingly.
// Time to verify hash should be as long as that can be tolerated.
// Cost 17: BenchmarkCompareHashAndPassword-8   	1000000000	        10.7 ns/op
func BenchmarkCompareHashAndPassword(b *testing.B) {
	// get password hasher
	hasher := GetPasswordHasher()

	// compare hash
	hasher.CompareHashAndPassword(testPasswordHash, testPassword)
}

func testNCharactersPasswordLimit(n int, t *testing.T) {
	// get password hasher
	hasher := GetPasswordHasher()

	// generate hash
	passwdhash, _ := hasher.GetPasswordHash(createPassword(n))

	ok, _ := hasher.CompareHashAndPassword(passwdhash, createPassword(n+1))

	Assert(t, !ok, fmt.Sprintf("[%v] and [%v] character passwords matched", n, n+1))
}

// bcrypt has password length limit of 72 characters. To test a new implementation, rename with "T" and set a range for i.
func testPasswordLengthLimit(t *testing.T) {
	for i := 71; i < 74; i++ {
		testNCharactersPasswordLimit(i, t)
	}
}
