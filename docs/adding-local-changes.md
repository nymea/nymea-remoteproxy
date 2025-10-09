# Updating an Open Pull Request with Local Changes

Follow these steps to add new local changes to the pull request that tracks the current branch.

1. **Check out the PR branch**
   ```bash
   git checkout <pr-branch>
   ```
2. **Make and verify your changes locally.** Build or run tests as needed.
3. **Stage the updates**
   ```bash
   git add <paths>
   ```
4. **Commit the changes**
   ```bash
   git commit -m "Describe the follow-up change"
   ```
5. **Push to the same remote branch**
   ```bash
   git push
   ```
   The pull request updates automatically with the new commits.
6. *(Optional)* **Amend or fix up the previous commit** if you prefer a single-commit PR.
   ```bash
   git commit --amend
   # or
   git commit --fixup <commit>
   git rebase -i --autosquash
   git push --force-with-lease
   ```
7. **Confirm the PR on GitHub (or your forge)** reflects the new commits and status checks.

> **Tip:** Run `git status` at any point to confirm which branch you are on and what is staged.
