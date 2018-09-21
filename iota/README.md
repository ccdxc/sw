##IO Test Automation

### Dev Instructions
1. Using vikasbd/sw:iota branch for all development
2. While raising PR ensure that the base branch is selected to vikasbd/sw:iota instead of master
3. Refrain from importing code from outside this directory. We want to reuse bits and pieces from pre existing infra, please copy it in. Once this is stabilized, we can delete all the unnecessary pieces of e2e stuff in one fell swoop.

## Code structure
1. All individual services are enumerated in iota/svcs
2. Helper scripts go in scripts/
3. Common protos go in protos/