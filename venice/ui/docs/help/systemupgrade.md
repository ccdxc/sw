---
id: systemupgrade
---

# System Upgrade

![System-Upgrade image](/images/system-upgrade.jpg)

Within the System Upgrade view, this allows the platform to be upgraded to a new software release.  The process is broken into 2 separate tasks.

- Rollout Images: This uploads the new firmware revision onto the Venice controllers
- Create Rollout: Create the upgrade "rollout" process to update the Venice controllers and DSCs adapters


## Rollout Images

### Upload Image File

- Choose Venice image file (*.tar) 
- Select "Upload" to start the process

Note: This will take a few minutes for the upload to complete

Once uploaded, this section will list the new available software version for upgrading under the "Images Repository"

## Create Rollout

This section defines how the software upgrade rollout will proceed. 

| Option | Description |
| ------ | ----------- |
| What to upgrade | DSCs Only, Venice Only or Both DSCs and Venice |
| Name of the rollout | Unique name for the rollout |
| Version | Which software version to upgrade to |
| Start Time | specific  start time or now|
| End Time | specific end time. If not specified, Venice will run upgrade until completion|
| Strategy | Linear or Exponential |
| Upgrade Type | Disuptive or On Next Host Reboot |
| Max DSC in Parallel | The number of Distributed Service Cards (DSCs) to be upgraded in parallel |
| Max Allowed DSCs failures | The total number of failed Distributed Service Card upgrade failures to occur before upgrade process is stopped |
| Retry for failed DSCs | If checked, Venice will retry upgrading DSCs up to 5 times before upgrade end time is reached. Requires Max Allowed DSC failures to be greater than 0. |
