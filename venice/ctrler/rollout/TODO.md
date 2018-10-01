Pending tasks

From Review:

* Prevent admission from Naples (even for auto-join) when Rollout is in progress - Changes in cmd
* Once the Rollout is in progress - no changes to cluster object  (say quorum nodes etc)should be allowed
* During PreCheck of SmartNICs, dont fail after maxFailures - Instead do preUpgrade on all the smartNICs in Cluster
    This is so that we collect all possible errors in one shot and report to customer
* Seperate out Download and PreCheck for the Naples. Possibly the preupgrade can happen with just the json file.
    May be call them: PreCheckBeforeDownload and PreCheckAfterDownload
* Rename Pause to Stop in Rollout Spec
* Rollout - check to see that only one of Rollout objects can be active at any time (and others are either in pending or
    completed state)  
* Work with sanjay on how to hide certain attributes of an object.
    e.g: bypassVeniceUpgrade knob
* Rename nmd_interfaces.go from API to something else.

Pending Implementation tasks:
* Implement Linear/Exponential scheme for upgrading naples
* Handle PreUpgrade Failures of smartNIC
* Implement MaxFailures 
* Implement Stop of Rollout
* Implement Delete of Rollout when the Rollout is in progress.
* Set the status in Rollout object correctly
* On restart of Rollout controller, restore Status/State from the Rollout Object and skip the upgrade of those that are 
    completed already.
* CMD should persist the venice Rollout state of the node at its level to provide idempotency.
    If a request comes to upgrade a venice node again, currently it asks systemd which will do a no-op. 
    Instead  investigate ways to bypass calls to systemd.
* Integration with Image download on the NAPLES
* Handling of image upload on venice
* Remove debug Rest endpoints in cmd after all the development is complete
 