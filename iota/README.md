##IO Test Automation

### Dev Instructions
1. Using vikasbd/sw:iota branch for all development
2. While raising PR ensure that the base branch is selected to vikasbd/sw:iota instead of master
3. Refrain from importing code from outside this directory. We want to reuse bits and pieces from pre existing infra, please copy it in. Once this is stabilized, we can delete all the unnecessary pieces of e2e stuff in one fell swoop.

## Code structure
1. All individual services are enumerated in iota/svcs
2. Helper scripts go in scripts/
3. Common protos go in protos/


## Building and using new workload images
1. Update box.rb in workload-images to add packages to iota workload image
2. Update the image tag in the Makefile under workload-images
3. Do make images from iota to create a new image
4. Do docker push of the newly created image (check docker images)
5. Modify svcs/common/constant.go to update the image tag
6. Create a PR and merge
