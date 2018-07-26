
## Directory Structure
- ci_targets: contains various e2e CI targets; each e2e target requires a separate job.yml so we separate them out in their individual directories
- e2e: e2e tests
- integ: integration tests for functional areas
- scale: scale tests
- utils: utilites to help with e2e and scale tests

## How to add an e2e test
Please read [this](./utils/infra.md) document to learn about writing e2e test and supported infrastructure for this
