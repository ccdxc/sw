package tserver

// PreCreateCallback allows an opportunity to modify/perform an operation an object before creation
func PreCreateCallback(obj interface{}, update bool, dryRun bool) error {
	return nil
}

// PreDeleteCallback alllows an opportunity to do any operation on the object before deletion
func PreDeleteCallback(obj interface{}, dryRun bool) error {
	return nil
}
