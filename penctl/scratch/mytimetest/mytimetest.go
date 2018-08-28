package main

import (
	"fmt"
	"time"
)

func main() {
	fmt.Printf("Yesterday: %+v\n", time.Now().Add(-24*time.Hour))
	fmt.Printf("Yesterday: %+v\n", time.Now().AddDate(0, 0, -1))

	mytimechan := time.After(5 * time.Second)
	currtime := <-mytimechan

	fmt.Printf("Yesterday: %+v\n", currtime)

	c := time.Tick(1 * time.Second)
	for now := range c {
		fmt.Printf("%v\n", now)
	}
}
