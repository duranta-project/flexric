<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# Repository Guidelines for Agents

These guidelines are the default conventions for working in this repository.
Explicit user instructions always take precedence and override them.

This file only adds what an agent needs on top of [CONTRIBUTING.md](./CONTRIBUTING.md),
which holds the licensing and contribution requirements. Read it first.

## Key directories

- `src/ric/`: nearRT-RIC, with the E42 interface to xApps in `src/ric/iApp/`.
- `src/agent/`: E2 agent library, linked by E2 nodes (the RAN or the emulators).
- `src/xApp/`: xApp SDK; Python bindings are generated with SWIG in `src/xApp/swig/`
  when built with `-DXAPP_MULTILANGUAGE=ON`.
- `src/sm/`: service models (KPM, RC, MAC, RLC, PDCP, GTP, slice, TC), with ASN.1,
  FlatBuffers, and/or plain encoders/decoders depending on the model.
- `src/lib/`: E2AP messages (per version, in `src/lib/e2ap/`), SCTP endpoints,
  and shared 3GPP information elements.
- `src/util/`: common data structures and configuration file parsing.
- `examples/`: the nearRT-RIC binary, E2 node emulators, and C/Python example xApps.
- `test/`: unit tests, registered with CTest.
- `docker/`, `openshift/`: image build files.
- `ci-scripts/`: CI files.

## Commit guidelines

Follow the [commit guidelines](./CONTRIBUTING.md#commit-guidelines). In particular:

- Agents MUST NOT add a `Signed-off-by` or a `Co-authored-by` tag. Only humans can
  legally certify the Developer Certificate of Origin (DCO).
- Agents MUST add an `Assisted-by` tag, so that the evolving role of AI in the
  development process stays visible:

      Assisted-by: AGENT_NAME:MODEL_VERSION

  where `AGENT_NAME` is the AI tool or framework and `MODEL_VERSION` the exact model
  version used, for example:

      Assisted-by: Claude:claude-opus-5

- Do not write long, overwhelming commit messages.
- Keep each commit focused on one logical change.
- Keep the subject concise, preferably no more than 72 characters.

Every commit must be signed with the SSH or GPG key of the human submitter, otherwise
the pull request is not merged. The key and its configuration belong to the developer,
so agents must not change them, and must never work around a failing signature with
`--no-gpg-sign`: report the failure instead. Check a commit with
`git log --show-signature`. See [signing commits](./CONTRIBUTING.md#signing-commits).

## Rebase the branches with the latest dev

Feature branches must be rebased on the latest `origin/dev` and must not contain
any merge commit. See
[this section](./CONTRIBUTING.md#keeping-your-branch-up-to-date) for more details.

## Builds

```bash
mkdir build && cd build && cmake .. && make -j8
```

The E2AP and KPM versions are selected at configure time
(`-DE2AP_VERSION=E2AP_V1|E2AP_V2|E2AP_V3`, `-DKPM_VERSION=KPM_V2_01|KPM_V2_03|KPM_V3_00`),
and version-specific code must be guarded accordingly. The CI builds and tests
`E2AP_V1`/`KPM_V2_01`, `E2AP_V2`/`KPM_V2_03` and `E2AP_V3`/`KPM_V3_00`, so check every
version your change touches. More details are in [README.md](./README.md#2-flexric-installation).

To build the image the CI builds, see
[Building Images Locally](./CONTRIBUTING.md#building-images-locally):

```bash
docker build --target oai-flexric --tag oai-flexric:test \
             --file docker/Dockerfile.flexric.ubuntu \
             --progress=plain .
```

## Tests

Agents should always try to add unit tests for new features and bug fixes. Tests live
under `test/` and are registered with `add_test()` in the matching `CMakeLists.txt`.

The agent/RIC/xApp tests (`test/agent-ric/`, `test/agent-ric-xapp/`) load the installed
configuration and service models (by default from `/usr/local/etc/flexric/` and
`/usr/local/lib/flexric/`), so install them before running the tests from the build
directory:

```bash
sudo make install
ctest -j8 --output-on-failure
```

The CI runs the same tests in `ci-scripts/Dockerfile.ctest`, except `Unit_test_TC_SM`.

## Actions that need explicit instructions

Agents must never do any of the following without being explicitly asked to:

- Push or update a remote or upstream branch, create a pull request, or post a
  comment on one.
- Force-push, or rewrite commits that are already pushed (rebase, amend, squash).
- Delete branches or tags, locally or on a remote, or create or push tags.
- Add or remove pull request labels.
- Push images to a container registry (Docker Hub or the private registry).
- Add or update third-party dependencies, or change license headers.
