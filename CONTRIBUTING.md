<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# Contribution Guidelines

We want to make contributing to this project as easy and transparent as possible.

1. Create an account on [GitHub](https://github.com/). Only contributions
   against [`duranta-project/flexric/`](https://github.com/duranta-project/flexric/)
   are accepted.
2. Fork the repository, and open pull requests for your contributions from your
   fork.
3. [Sign the CLA](https://github.com/duranta-project/governance/blob/main/docs/easy_cla_process.md)
   either before making your first pull request or after submitting the
   pull request.
4. All commits must be signed, and the commit author email must match the email
   address associated with the CLA.

## Commit Guidelines

Every pull request must pass the required CI checks before it can be merged:

1. **[Developer Certificate of Origin (DCO)](https://en.wikipedia.org/wiki/Developer_Certificate_of_Origin)**:
   Each commit must include a `Signed-off-by:` trailer in the commit message.
   Use `git commit -s` (or `--signoff`).

2. **[Verified commits](https://docs.github.com/en/authentication/managing-commit-signature-verification/about-commit-signature-verification)**:
   Each commit must be cryptographically signed using SSH or GPG keys to confirm
   its origin.

### Signing Commits

GitHub supports commit signing using either SSH keys or GPG keys.
For more information, see the
[GitHub documentation](https://docs.github.com/en/authentication/managing-commit-signature-verification/signing-commits).

Before configuring commit signing:

- Generate an SSH key pair or GPG key pair.
- Add your public key to your GitHub account.
- Verify your GitHub email address (required for “Verified” commits to work).
- If using SSH signing, ensure the key is registered in GitHub for:
    - Authentication (SSH and GPG keys)
    - Signing commits (Signing Keys)

> [!NOTE]
> Adding an SSH key for repository access does not automatically enable commit signing.
> The key must also be added under GitHub's **Signing Keys** settings.


To ensure commits show as Verified on GitHub:

- Your `git config user.email` must match a GitHub email
- That email must be verified in your GitHub account

For more information, see the
[GitHub Docs](https://docs.github.com/en/account-and-profile/how-tos/email-preferences/verifying-your-email-address)

Configure your repository's `.git/config`:

```ini
# Edit the git configuration

[user]
    name = YOUR NAME
    email = YOUR VERIFIED EMAIL ADDRESS

    # REQUIRED for commit signing
    # Use ONE signing method (SSH or GPG)

    signingkey = YOUR_SIGNING_KEY

    # Examples:
    # SSH signing:
    # signingkey = ~/.ssh/id_ed25519.pub

    # GPG signing:
    # signingkey = YOUR_GPG_KEY_ID

[gpg]
    # REQUIRED: defines signing method (SSH or GPG)

    format = YOUR_SIGNING_FORMAT

    # Examples:
    # SSH signing:
    # format = ssh

    # GPG signing:
    # format = openpgp

[commit]
    gpgsign = true
```

> [!NOTE]
> The private key is used automatically by Git when signing commits.
> The private key should never be shared.

#### Verifying Signed Commits

You can verify that commits are properly signed locally using:

```bash
git log --show-signature
```

GitHub should also display a Verified badge next to signed commits once the
signing key has been correctly configured in your account.

##### SSH Signature Verification (`allowed_signers`)

For SSH commit signing, local Git verification may require an `allowed_signers`
file. This is only used for local verification in Git and is not required
by GitHub.

If you see errors such as:

```text
No principal matched
Can't check signature
error: gpg.ssh.allowedSignersFile needs to be configured
```
you may need to configure it.

Create the file and add your signing identity:

```bash
mkdir -p ~/.config/git
touch ~/.config/git/allowed_signers
echo "user@example.com ssh-ed25519 AAAACexamplekeystringhere" > ~/.config/git/allowed_signers
```

Enable it in local repository Git config:

```bash
git config gpg.ssh.allowedSignersFile ~/.config/git/allowed_signers
```

> [!NOTE]
> This is only for local Git signature verification and does not affect GitHub
> or remote repository behavior.

> [!IMPORTANT]
> If your commits are not signed, the pull request will not be merged.

### Writing Commit Messages

To help maintainers and reviewers understand your changes, please follow these guidelines when writing commit messages:

- Use short, descriptive title
- Separate subject from body with a blank line
- Use the body to explain what and why
- Prefix your commit with a type, for example:

```bash
feat: Add a new feature
fix: Fix a bug
refactor: Rewrite or restructure code without adding a feature or fixing a bug
chore: Update dependencies or perform miscellaneous maintenance
perf: Improve performance
ci: Update Continuous Integration workflows
docs: Update documentation (README, tutorials)
style: Apply code formatting changes (whitespace, indentation, etc.)
test: Add or fix tests
```

In case you make an error in a recent commit you can run the following command:

```bash
git commit --amend # allows you to modify and add changes to the most recent commit
git push origin feature-branch --force-with-lease
```

#### Use of git commit trailers

You have to sign all your commits. Thus, every commit must have a git commit trailer that reads

```
Signed-off-by: Full Name <email-for-cla>
```

There are additional commit trailers that you can or should use:

- `Assisted-by: <Name>:<model>`: if you have been assisted by an AI/LLM, you
  must disclose this by indicating both the LLM name and model. Note that LLMs
  do not author, as _the submission is under your name_ (i.e., NEVER add an LLM
  through `Co-authored:by:`). The [Linux kernel documentation on AI
  assistants](https://docs.kernel.org/process/coding-assistants.html)
  might be helpful.
- `Reviewed-by: Full Name <email>` for a person that reviewed a code. We attach
  this trailer to the merge commit for people that reviewed a pull request.
- `Co-authored-by: Full Name <email>` for a person that significantly
  contributed to a commit and has co-authorship.
- `Fixes: <commit> ("<title>")` if a given commit fixes bug in an earlier,
  referenced commit. For ease-of-use, please include the commit title, and only
  the commit SHA, not a link.
- `Closes: #Issue` if a specific commit closes a bug. If the pull request
  description includes this, we add this to the merge commit.
- `Reported-by: Full Name <email>` if a person reported a bug or other useful
  information that led to this commit.
- `Tested-By: Full Name <email>` if a person tested a given patch.

Please also check the documentation via `man git-interpret-trailers`

#### AI Assistants

These guidelines are mostly based on [linux kernel guidelines](https://docs.kernel.org/process/coding-assistants.html)

This document provides guidance for AI tools and developers using AI assistance
when contributing to the repository.

AI tools helping with the development should follow the standard
Duranta developement procedure.

- All code must be compatible with CSSL v1.0
- Use appropriate SPDX license identifiers

AI agents MUST NOT add `Signed-off-by` nor `Co-authored-by` tags.  Only humans
can legally certify the Developer Certificate of Origin (DCO).  The human
submitter is responsible for:

- Reviewing all AI-generated code
- Ensuring compliance with licensing requirements
- Adding their own Signed-off-by tag to certify the DCO
- Taking full responsibility for the contribution

When AI tools contribute to Duranta,
proper commit message helps track the evolving role of AI in the development process.
Contributions should include an `Assisted-by` tag in the following format:

    Assisted-by: AGENT_NAME:MODEL_VERSION

- `AGENT_NAME` is the name of the AI tool or framework
- `MODEL_VERSION` is the specific model version used

```
Example:

    Assisted-by: Claude:claude-opus-5
```

### Rewriting Commits

Your commit history should remain clean and meaningful. Avoid commits that only “clean up” or fix issues in previous commits, such as messages like `Fix typo`.
Instead, combine those changes into a single commit using interactive rebase or fixup commits.

- Use `git rebase -i` to interactively edit older commit messages or squash related commits.
- Use `git commit --fixup=<commit-hash>` to mark a commit for automatic squashing into a previous commit.
- Please make sure that the commit message clearly summarizes all changes included.

**Example using Interactive Rebase:**

```bash
git rebase -i HEAD~3   # interactively rebase the last 3 commits
# mark commits to squash with "s" or "squash" and edit the final commit message
# mark commits to edit commit message with "e" or "edit"
# save and follow the prompts to update messages
git push origin feature-branch --force-with-lease # force with lease let's you only overwrite what you also have locally in origin/feature-branch
```

**Example using fixup commits:**

```bash
# Create a fixup commit to automatically squash into an earlier commit
git commit --fixup=<commit-hash>

# Start an interactive rebase with autosquash
git rebase -i --autosquash <commit-hash>^

# Git opens a commit list:
## pick - keep the commit as-is
## fixup - automatically squash into the previous commit
## If you save and close the file with no other changes, the rebase will proceed

# If conflicts occur during the rebase, resolve them and run
git rebase --continue

# Push to remote branch
git push origin feature-branch --force-with-lease
```

### Keeping Your Branch Up to Date

**Do not merge `dev` into your branch.** The CI rejects any pull request whose source
branch contains a merge commit:

```
Error: Following merge commits are found in the source branch history. Please rebase your branch.
```

To pick up changes from `dev`, rebase on top of it instead:

```bash
git fetch origin
git rebase origin/dev
git push --force-with-lease
```

This keeps the history linear and every commit reviewable on its own.

## Building Images Locally

The CI builds a FlexRIC image for Ubuntu and for CentOS Stream. Building them locally is
a quick way to check that your changes break neither:

```bash
# Ubuntu
docker buildx build --no-cache --target oai-flexric --tag oai-flexric:latest \
  --file docker/Dockerfile.flexric.ubuntu .
# CentOS Stream
docker buildx build --no-cache --target oai-flexric --tag oai-flexric:latest \
  --file docker/Dockerfile.flexric.centos .
```

The E2AP and KPM service model versions are build arguments, so you can build the image
against a version other than the default:

```bash
docker buildx build --target oai-flexric --tag oai-flexric:latest \
  --build-arg E2AP_VERSION=E2AP_V2 --build-arg KPM_VERSION=KPM_V2_03 \
  --file docker/Dockerfile.flexric.ubuntu .
```

> Note that the CI builds the Ubuntu image for both `linux/amd64` and `linux/arm64`, while
a local build produces an image only for your own architecture.

The README describes how to [run the images as a testbed](./README.md#42-opt-docker-testbed).

## Main Workflow

### 1. Push your changes to a new branch

Push your modified code to a new branch in the [GitHub repository](https://github.com/duranta-project/flexric).

* Please use a short and descriptive branch name.
* Keep your branch up to date with `dev`.

### 2. Open a pull request

Create a pull request on [GitHub](https://github.com/duranta-project/flexric/pulls).

* The `target` (`base` in the GitHub interface) branch **must be `dev`**.
* The `source` (`compare` in the GitHub interface) branch is your development branch.
* Break large changes into smaller, logical commits and keep pull requests focused.
  Smaller pull requests are easier to review, test, and merge.
* Add the https://github.com/duranta-project/flexric/labels/ci-build-test label when opening the pull request
  to run the build and test suites.
* For documentation-related changes only, add the https://github.com/duranta-project/flexric/labels/documentation
  label.
* A pull request must have either the https://github.com/duranta-project/flexric/labels/ci-build-test or https://github.com/duranta-project/flexric/labels/documentation label for CI to run.
* If the required label is not added, CI will not run.
* If you do not have access to add labels, request a maintainer or project member to add
  one for your pull request.

You might set a pull request in "draft" to signal that the pull request is not ready for
review, but the CI will still run if the https://github.com/duranta-project/flexric/labels/documentation or
https://github.com/duranta-project/flexric/labels/ci-build-test label is present.
To communicate that a pull request is ready, simply
mark it as "ready". Of course, you are invited to reach out to the reviewer(s) through
other means to coordinate the review work.

### 3. Review the CI results

The Continuous Integration (CI) process will be triggered and will validate your changes:

For all pull requests with a required label:

* Verify contribution requirements (e.g., signed commits)

For pull requests with the https://github.com/duranta-project/flexric/labels/ci-build-test label:

* Build images
* Run test suites

The CI posts the results in the comments section of the pull request. Both pull request
authors and reviewers are responsible for manual inspection and pre-filtering of the CI
results.

### 4. Fix failing CI checks

If any CI check fails, push the required fixes to your source branch.

* Before pushing new commits, group related fixes together and test them locally when
  possible (see [Building Images Locally](#building-images-locally)).
* Avoid pushing multiple intermediate commits for the same CI failure.
* CI will automatically run again on the new commit as long as the required
  labels are present.
* Please wait for the current CI run to complete before pushing additional changes.
* This helps ensure fair CI resource usage for all contributors.

If the CI failed because of a temporary issue rather than your changes, add the
https://github.com/duranta-project/flexric/labels/retrigger-ci label to re-run it
without pushing a new commit. This label is removed automatically once a run
starts, so the label can be applied again for each re-run.

### 5. Respond to the review

Once all CI checks pass, a maintainer will review your changes or assign them to a senior
contributor for peer review.

* The reviewer will check the code, commit messages, and CI results.
* All review discussions must be resolved before approval.

Once you addressed comments and pushed new changes, please answer for each comment how it
has been addressed, and consider writing a general "overview" comment highlighting the
changes that you have applied. This helps the reviewer get a quick idea of what has
changed and what needs to be checked. Note that the bigger the changes, the more such
summary is necessary, and will make follow-up review easier and hence more likely to
actually happen.

### 6. Approval and merge

After approval, a CI administrator will merge the pull request.

* CI will run again on the updated `dev` branch.
* The source branch will be deleted after the merge.

## License

By contributing to Duranta, you agree that your contributions will be
licensed under

1. [CSSL v1.0 license](LICENSES/preferred/CSSL-v1.0.txt): for flexric
   related source code and test scripts
2. [CC-BY-4.0](LICENSES/preferred/CC-BY-4.0.txt): All documentation
3. [MIT](LICENSES/preferred/MIT.txt): Orchestration (helm-charts, docker
   compose), ci-scripts and generic source code (for example, `src/util/alg_ds/*`)

Certain files are using different licenses; you can read about them in
[NOTICE](NOTICE).

## Reporting Bugs

Please report software bugs or security issues through the [GitHub Issue Tracker](https://github.com/duranta-project/flexric/issues).

Add the https://github.com/duranta-project/flexric/labels/bug label for a software defect,
or the https://github.com/duranta-project/flexric/labels/security label for a security
issue. If you do not have access to add labels, request a maintainer or project member
to add one for your issue. If you are unsure whether your report is a bug, start a
discussion through the [mailing list](https://github.com/duranta-project/flexric/wiki/MailingList).

If required, the maintainers will create an issue on your behalf.

When reporting an issue, please include:

* A clear description of the problem
* Expected behavior — what you expected to happen
* Observed behavior — what actually happened
* Steps to reproduce the issue, including commands, configuration files, or environment details when applicable
* Logs or command outputs using bullet points and code blocks to make the information easier to review
