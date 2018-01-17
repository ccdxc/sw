### Vendoring considerations
We want vendoring tools to do the following.
- Easy way to add newer dependencies.
- Have semver based, declarative approach to listing dependencies.
- Easy upgrade paths to ensure security patches are applied.
- Single source of truth and one place to list all the 3rd party code.
- Use widely supported, stable package management tools.

### Case for dep
[dep](https://github.com/golang/dep) meets all of the above requirements reasonably
well.

### To commit or not commit the vendor folder
dep can successfully build and pass all venice and agent tests. However dep ensure
pulls in entire vendored repos, wherther we actually import the packages or not.
However there is `dep prune` which removes all unused packages. The builds and tests are currently
working even on the pruned version of vendor. The tool is [heading towards absorbing](https://github.com/golang/dep/issues/944)
`dep prune` into `dep ensure`

### Committing entire vendor folder
`Pros:`
- Truly 100% reproducible builds. Guards against upstream master force pushes.
- go install can be directly called without any workflow/makefile changes.

`Cons:`
- Large repo. Currently 327M. It will only get bigger.
- PR Fatigue. This will result in extra long PRs

### .gitignoring vendor
`Pros:`
- Repo will have only pensando code. Faster clone, performance.
- Slight nudge towards guarding against, directly updating vendor code and forgetting to update the manifests.

`Cons:`
- Vulnerable towards breaking upstream master changes.

### Committing only Pruned Vendor
`Pros:`
- Most balanced approach of the above two.

`Cons:`
- Currently have issues with vendored CI code. `asset-build` tools have trouble currently with some of the transitive dependencies.
Preferably CI binaries can be hosted on its own github repo with tracked releases.

### Sample Workflows
#### Adding newer dependencies
- `dep ensure -add github.com/foo/bar`
- import github.com/foo/bar in your code.
- Run `dep prune` along with this. [Future releases of dep will not need this](https://github.com/golang/dep/issues/944)
- Commit `Gopkg.lock`, vendored code and `Gopkg.toml`

#### Updating existing dependencies
- Update the semver in `Gopkg.toml`
- Run `dep ensure`
- Run `dep prune`. [Future releases of dep will not need this](https://github.com/golang/dep/issues/944)
- Commit `Gopkg.lock`, vendored code and `Gopkg.toml`

#### Removing dependencies
- Remove the imports from source.
- Run `dep ensure`
- Run `dep prune` [Future releases of dep will not need this](https://github.com/golang/dep/issues/944)
- Commit `Gopkg.lock`, vendored code and `Gopkg.toml`

#### Working with pensando forks.
- Submit a PR for the upstream fork and get it merged.
- Run `dep ensure -update github.com/pensando/foo`
- Run `dep prune`
- Commit `Gopkg.lock`, vendored code and `Gopkg.toml`

#### Semantic versioning cheatsheet
It is recommended to use semantic versioning for the imported packages.
Also `dep` by default uses `^` operator if no other operators are specified.
If `github.com/foo` is defined as follows
```
[[constraint]]
  name = "github.com/foo"
  version = "1.6"
```

`^` will interpret the git version as the min-boundary ranges like below.
- `1.6`   translates to the highest compatible commit available between `>=1.6.0 to < 2.0.0`
- `0.6.2` translates to the highest compatible commit available between `>=0.6.2 to < 0.7.0`
- `0.0.4` translates to the highest compatible commit available between `>=0.0.4 to < 0.1.0`

`dep` [uses an external semver library](https://github.com/Masterminds/semver)
It has support for [fancier operators and regexes](https://github.com/golang/dep#semantic-versioning)

However if our code is still compatible with `v2.0.0` and higher, just doing a dep ensure will *not*
update `github.com/foo`, We will need a corresponding `Gopkg.toml` change to the constraint as follows

```
[[constraint]]
  name = "github.com/foo"
  version = "2.0"
```

### Things to consider while adding newer packages
- Preferably use a released version of the dependency which complies with semantic versioning
- Avoid using different versions of the same repo. For eg. If you need a go package, github.com/foo/a
and github.com/foo/b, consider using a single released version of github.com/foo. `dep prune` will take
care of prunuing unused packages.
- Avoid using github.com/Sirpusen/logrus, the uppercase version as it is [deprecated](https://github.com/sirupsen/logrus)
- Please ensure that any changes in the checked in vendor code has a corresponding, matching entry in `Gopkg.toml` and `Gopkg.lock`

### A note on nested Vendoring
`dep` doesn't do nested vendoring. It is one of its [fundamental design goals](https://github.com/golang/dep/issues/985#issuecomment-321353370)
`dep` will ensure that the nested vendoring is stripped out and flattened.
The transitive dependencies will be captured in `Gopkg.lock`

## What to do when dep's vendor flattening causes issues.
Having said that there is a small possibility that nested vendoring can cause incompatible changes
if the upstream repo is not actively maintained. Here are some mitigation we can do
to shield ourselves from this. If `github.com/foo/bar` needs `1.0` of `github.com/foo/a` and `github.com/foo/baz` `1.2` of needs `github.com/foo/a`, dep usually does a good job of figuring compatible commits. When it fails to do this here are some things
we can do to sheild ourselves.

- Find a compatible commit between 1.0 and 1.2 which satisfies github.com/foo/bar and github.com/foo/baz
- If that fails, Use an override version for the offending package. This basically tells dep to freeze on that version and then try to get compatible versions for other packages.
- If all else fails maintain a private fork by cherry picking required changes from the thid party code. (Usually we will never have to do this. The pains of doing this can far outweigh its benefits)

PS: Considering the whole community is going towards `dep` and its flattened vendors, it is a fair assumption to expect that the
packages will keep in sync. Having a [nested vendored tree can cause a lot of suffering](https://groups.google.com/forum/#!msg/golang-nuts/AnMr9NL6dtc/UnyUUKcMCAAJ)
A combination of picking released versions of vendored code, and overriding dependencies is usually the ideal approach.
