package libs

import (
	"encoding/binary"
	"fmt"
	"math"
	"math/rand"
)

func GenMACAddresses(count int) ([]string, error) {
	// Fixed seed to generate repeatable IP Addresses
	var macAddresses []string
	//numbers := make([]int64, count)

	for i := 0; i < count; i++ {
		b := make([]byte, 8)
		num := uint64(rand.Int63n(math.MaxInt64))
		binary.BigEndian.PutUint64(b, num)
		// HAL will complain if EP has a multicast MAC Address. This will ensure that we have unicast MAC
		b[0] = (b[0] | 2) & 0xfe
		mac := fmt.Sprintf("%02x:%02x:%02x:%02x:%02x:%02x", b[0], b[1], b[2], b[3], b[4], b[5])
		macAddresses = append(macAddresses, mac)
	}

	return macAddresses, nil
}
