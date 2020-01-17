package state

// CreateLateralNetAgentObjects is a stubbed out method for non linux systems.
func (na *Nagent) CreateLateralNetAgentObjects(owner string, mgmtIP, destIP string, tunnelOp bool) error {
	return nil
}

// DeleteLateralNetAgentObjects is a stubbed out method for non linux systems.
func (na *Nagent) DeleteLateralNetAgentObjects(owner string, mgmtIP, destIP string, tunnelOp bool) error {
	return nil
}
