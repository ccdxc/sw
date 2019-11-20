---
id: systemupgrade
---

# System Upgrade

![System-Upgrade image](/images/system-upgrade.jpg)

Within the System Upgrade view, this allows the platform to be upgraded to a new software release.  The process is broken into 2 separate tasks.

- Rollout Images: This uploads the new firmware revision onto the Venice controllers
- Create Rollout: Create the upgrade "rollout" process to update the Venice controllers and NAPLES adapters


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
| What to upgrade | NAPLES Only, Venice Only or Both NAPLES and Venice |
| Name of the rollout | Unique name for the rollout |
| Version | Which software version to upgrade to |
| Scheduling | specific time or now |
| Strategy | Linear or Exponential |
| Upgrade Type | Disuptive or On Next Host Reboot |
| Max DSC in Parallel | The number of Distributed Service Cards (NAPLES) to be upgraded in parallel |
| Max Allowed DSCs failures | The total number of failed Distributed Service Card upgrade failures to occur before upgrade process is stopped |

