package datapath

// GetUUID gets the FRU information for the NAPLES from HAL.
func (hd *Datapath) GetUUID() (string, error) {

	//// Get the
	//if hd.Kind == "hal" {
	//	resp, err := hd.Hal.SystemClient.SystemUuidGet(context.Background(), &halproto.Empty{})
	//	if err != nil {
	//		log.Errorf("Error creating next hop. Err: %v", err)
	//		return "", err
	//	}
	//	if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
	//		log.Errorf("HAL returned non OK status. %v", resp.ApiStatus.String())
	//		return "", fmt.Errorf("HAL returned non OK status. %v", resp.ApiStatus.String())
	//	}
	//	return resp.Uuid, nil
	//}

	// Mock HAL
	return hd.getDefaultUUID()
}
