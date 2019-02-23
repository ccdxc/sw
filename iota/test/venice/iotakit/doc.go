// Package iotakit is a wrapper library for writing iota tests.
// This library provides an abstract model of the system under test. Tests can configure venice or perform actions on the cluster by manipulating this abstract model called "SysModel".
//
// Most of the logic of changing configuration, verifying status and performing triggers is done by the SysModel. SysModel allows us to write test cases using high level primitives instead of object CRUD primitives. SysModel also has a very expressive query language to iterate over the model and perform operations on them.
//
// See SysModel for more details
package iotakit
