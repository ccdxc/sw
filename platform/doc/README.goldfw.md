This readme provides the steps to update the goldfw on minio to release newer goldfw

There are two artifacts related to goldfw and they go together.
1. Naples goldfw: This is the firmware that goes in Naples card
2. Gold FW compatible host drivers: This is the host side drivers (Linux, FreeBSD, ESX etc.) which are compatible with this goldfw (artifact #1). aka gold host drivers

Steps to release goldfw to minio:
1. The nic/goldfw job in jobd builds the goldfw and gold host driver as part of the hourly build job.
2. Find the Release Tag number in jobd for the hourly build from which you want to release the gold fw and gold host drivers
3. Gold fw can be found under this path: /vol/builds/hourly/< jobd release tag >/usr/src/github.com/pensando/sw/nic/buildroot/output_gold/images/naples_goldfw.tar
 and gold host drivers can be found under /vol/builds/hourly/< jobd release tag >/usr/src/github.com/pensando/sw/platform/gold_drv.tar.gz
4. Copy the naples_goldfw.tar from jobd artifact directory to pensando/sw/platform/goldfw/naples/ directory as naples_fw.tar filename.
5. rm -rf pensando/sw/platform/hosttools directory
6. Extract gold_drv.tar.gz artifact from jobd artifact under pensando/sw/platform directory. It will extract as hosttools directory.
7. Bump up the version of goldfw and gold host driver under pensando/sw/minio/VERSION. (bump up pensando_firmware version for gold firmware and host_tools version for gold host drivers)
8. Run this command to upload the new goldfw and gold host drivers to minio: cd pensando/sw/ && UPLOAD=1 ./scripts/create-assets.sh
9. Commit the changes in pensando/sw/minio/VERSION file and push to github.com
10. Create PR with this change in pensando/sw/minio/VERSION file and get it merged to master of pensando/sw repo. Once the PR is merged, everyone will have new goldfw and new gold host drivers when the rebase their worskspace with master branch.



