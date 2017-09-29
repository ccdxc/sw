package statssim

import (
	"fmt"
	"github.com/pensando/sw/venice/utils/log"
	"math/rand"
	"strings"
)

var (
	verbose = false

	actionList = []string{"Permit", "Deny", "Learn"}
	protoList  = []string{"tcp", "udp"}

	groupSize  []minMax
	globalVlan = int(1)
)

type minMax struct {
	min int
	max int
}

// EP represents an endpoint
type EP struct {
	// IPAddr of the endpoint
	IPAddr string
	// SvcPorts of the endpoint
	SvcPorts []string
	// Neighbors of the ep
	Neighbors []string
	// ParentGroup of the ep
	ParentGroup *Group
	// Vlan assigned to the ep
	Vlan string
}

// Group represents an ep group
type Group struct {
	// Height of this group in the tree
	Height int
	// Members of this group
	Members []TreeNode
	// Vlan assigned to this group
	Vlan int
	// ParentGroup of this group
	ParentGroup *Group
}

// TreeNode can be an endpoint or a group
type TreeNode interface {
	Discover(int, int) []string
}

// StatsSim simulates a set of endpoints organized into multi-tier apps.
type StatsSim struct {
	epList       []*EP
	maxFlowIndex int
	height       int // Height of app tree
	endpoints    int // Number of endpoints to simulate
	groupMin     int // Min size of group
	groupMax     int // Max size of group
	svcPorts     int // Number of service ports to simulate
}

// NewStatsSim returns an instance of StatsSim with default settings
func NewStatsSim() *StatsSim {
	return &StatsSim{
		height:    2,
		endpoints: 1000,
		groupMin:  8,
		groupMax:  16,
		svcPorts:  2,
	}
}

// Init sets up the simulator
func (s *StatsSim) Init() {
	groupSize = make([]minMax, s.height)
	min := s.groupMin
	max := s.groupMax
	for ix := 0; ix < s.height; ix++ {
		mm := minMax{min: min, max: max}
		groupSize[ix] = mm
		min = min >> 1
		max = max >> 1
	}

	s.epList = make([]*EP, 0, s.endpoints) // pre-alloc
	root := &Group{}
	log.Infof("Initializing %d endpoints, height %d", s.endpoints, s.height)
	root.Populate(s.height, 0, s.endpoints, s.svcPorts, &s.epList)
	for _, ep := range s.epList {
		ep.Connect()
	}
	s.maxFlowIndex = len(s.epList) - 1
	log.Infof("epList length: %d", len(s.epList))
}

// GetFlowStats returns a stats point from the simulator
func (s *StatsSim) GetFlowStats() (map[string]string, map[string]interface{}) {
	f := SafeRandInt(s.maxFlowIndex)
	return s.epList[f].TalkSimple()
}

// Populate populates a group with endpoints
func (g *Group) Populate(h, vlan, epCount, svcPorts int, epList *[]*EP) {

	defer func() {
		if verbose {
			fmt.Printf("\tHeight: %d members: %d\n", g.Height, len(g.Members))
		}
	}()

	g.Height = h
	g.Vlan = vlan

	if h == 0 {
		if epCount > 250 {
			log.Fatal("epCount needs to be less than 250, got ", epCount)
		}

		n1 := 10 + SafeRandInt(190)
		n2 := vlan/32 + 5
		n3 := vlan % 32
		for ix := 0; ix < epCount; ix++ {
			ipaddr := fmt.Sprintf("%d.%d.%d.%d", n1, n2, n3, ix+1)
			ports := []string{}
			portCount := SafeRandInt(5)
			for jx := 0; jx < portCount; jx++ {
				p := fmt.Sprintf("%d", 8100+SafeRandInt(svcPorts))
				ports = append(ports, p)
			}
			ports = append(ports, "8000")

			ep := &EP{
				IPAddr:      ipaddr,
				SvcPorts:    ports,
				ParentGroup: g,
				Vlan:        fmt.Sprintf("%d", vlan),
			}
			g.Members = append(g.Members, ep)
			*epList = append(*epList, ep)
		}

		return
	}

	if h > len(groupSize) {
		log.Fatal("Height is less than available group sizes")
	}
	h--

	for epCount > 0 {
		if h == 0 {
			vlan = getVlan() // kludge: allocating only for bottom level
		}

		currCount := groupSize[h].min + SafeRandInt(groupSize[h].max-groupSize[h].min)
		currCount = smallerOf(currCount, epCount)
		group := &Group{ParentGroup: g}
		group.Populate(h, vlan, currCount, svcPorts, epList)
		g.Members = append(g.Members, group)
		epCount -= currCount
	}
}

// Discover finds the neighborhood afforded by a group to its members
func (g *Group) Discover(intCount, extCount int) []string {
	result := make([]string, 0, intCount+extCount)
	if g.ParentGroup == nil {
		intCount += extCount // kludge
		extCount = 0
	}

	// discover internal neighbors
	maxIndex := len(g.Members) - 1
	for intCount > 0 {
		var currCount int
		if g.Height == 0 {
			currCount = 1
		} else {
			currCount = 1 + SafeRandInt(intCount)
		}

		currCount = smallerOf(currCount, intCount)
		m := g.Members[SafeRandInt(maxIndex)]
		result = append(result, m.Discover(currCount, 0)...)
		intCount -= currCount
	}

	if extCount != 0 {
		c1 := SafeRandInt(extCount)
		c1 = largerOf(c1, 1)

		result = append(result, g.ParentGroup.Discover(c1, extCount-c1)...)
	}

	return result
}

func smallerOf(x, y int) int {
	if x < y {
		return x
	}

	return y
}

func largerOf(x, y int) int {
	if x < y {
		return x
	}

	return y
}

// Discover finds the services provided by an ep to a neighbor
func (e *EP) Discover(intCount, extCount int) []string {
	result := make([]string, 0, intCount)

	for ix := 0; ix < intCount; ix++ {
		maxIndex := len(e.SvcPorts) - 1
		p := smallerOf(maxIndex, ix)
		t := e.IPAddr + ":" + e.SvcPorts[p]
		result = append(result, t)
	}

	return result
}

// Connect establishes the neighbor list of a given ep
func (e *EP) Connect() {
	// Assume connectivity size similar to group size
	count := groupSize[0].min + SafeRandInt(groupSize[0].max-groupSize[0].min)

	intCount := count >> 1

	extCount := SafeRandInt(intCount)
	intCount += intCount - extCount

	e.Neighbors = e.ParentGroup.Discover(intCount, extCount)

	if verbose {
		fmt.Printf("Self: %s Neighbors: %+v\n", e.IPAddr, e.Neighbors)
	}
}

// TalkSimple simulates a random connection to one the ep's neighbors
func (e *EP) TalkSimple() (map[string]string, map[string]interface{}) {
	flow := make(map[string]string)
	stats := make(map[string]interface{})

	flow["fwAction"] = "Learn"
	flow["ipProto"] = "TCP"
	flow["srcIP"] = e.IPAddr
	maxIndex := len(e.Neighbors) - 1
	ix := SafeRandInt(maxIndex)
	d := strings.Split(e.Neighbors[ix], ":")
	flow["destIP"] = d[0]
	flow["destPort"] = d[1]
	flow["vlan"] = e.Vlan

	stats["Rcv Bytes"] = SafeRandInt(16000)
	stats["Tx Bytes"] = SafeRandInt(19000)

	return flow, stats
}

func getVlan() int {
	globalVlan++
	if globalVlan > 4095 {
		log.Fatal("max vlans exceeded")
	}

	return globalVlan
}

// SafeRandInt returns a random int
func SafeRandInt(x int) int {

	if x < 0 {
		log.Fatal("negative number")
	}

	if x == 0 {
		return x
	}

	return rand.Intn(x)
}
