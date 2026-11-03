#!/usr/bin/env bash
set -euo pipefail

# Publish the committed leaderboard subtree without replacing the existing site.
mode="${1:---dry-run}"
if [[ $# -gt 1 || ( "$mode" != "--dry-run" && "$mode" != "--publish" ) ]]; then
  printf 'Usage: bash scripts/publish-leaderboard.sh [--dry-run|--publish]\n' >&2
  exit 2
fi

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(git -C "$script_dir" rev-parse --show-toplevel)"
source_branch="$(git -C "$repo_root" symbolic-ref --quiet --short HEAD || true)"
if [[ "$source_branch" != "leaderboard" ]]; then
  printf 'Switch to the leaderboard branch before publishing.\n' >&2
  exit 1
fi
if ! git -C "$repo_root" diff --quiet HEAD -- leaderboard/ ||
   [[ -n "$(git -C "$repo_root" ls-files --others --exclude-standard -- leaderboard/)" ]]; then
  printf 'Commit all leaderboard/ changes before publishing. Only committed files are deployed.\n' >&2
  exit 1
fi
git -C "$repo_root" cat-file -e HEAD:leaderboard/index.html
git -C "$repo_root" cat-file -e HEAD:leaderboard/data.json

source_sha="$(git -C "$repo_root" rev-parse HEAD)"
remote_url="$(git -C "$repo_root" remote get-url origin)"
case "$remote_url" in
  https://github.com/horizon-llm/IOI2026.git|https://github.com/horizon-llm/ioi2026.git|git@github.com:horizon-llm/IOI2026.git|git@github.com:horizon-llm/ioi2026.git) ;;
  *) printf 'The origin remote must be horizon-llm/IOI2026.\n' >&2; exit 1 ;;
esac

node --test "$repo_root"/tests/*.test.mjs

publish_tmp="$(mktemp -d "${TMPDIR:-/tmp}/ioi-leaderboard-publish.XXXXXX")"
trap 'rm -rf -- "$publish_tmp"' EXIT
git clone --quiet --single-branch --branch gh-pages "$remote_url" "$publish_tmp/site"
mkdir "$publish_tmp/source"
git -C "$repo_root" archive "$source_sha" leaderboard/ | tar -x -C "$publish_tmp/source"
if [[ -L "$publish_tmp/site/leaderboard" ]]; then
  printf 'Refusing to replace a symbolic-link leaderboard directory.\n' >&2
  exit 1
fi
mkdir -p "$publish_tmp/site/leaderboard"
rsync -a --delete "$publish_tmp/source/leaderboard/" "$publish_tmp/site/leaderboard/"
git -C "$publish_tmp/site" add --all -- leaderboard/

if git -C "$publish_tmp/site" diff --cached --quiet; then
  printf 'The committed leaderboard is already present on gh-pages.\n'
  exit 0
fi

git -C "$publish_tmp/site" diff --cached --stat -- leaderboard/
if [[ "$mode" == "--dry-run" ]]; then
  printf '\nDry run complete. No remote changes made. Run with --publish to deploy this subtree.\n'
  exit 0
fi

# Honor the source checkout's commit identity, including repository-local config.
git -C "$publish_tmp/site" config user.name "$(git -C "$repo_root" config user.name)"
git -C "$publish_tmp/site" config user.email "$(git -C "$repo_root" config user.email)"
git -C "$publish_tmp/site" commit -m "Publish leaderboard from ${source_sha:0:12}"
# A concurrent gh-pages update rejects this normal push. Rerun to start from its new head.
git -C "$publish_tmp/site" push origin HEAD:gh-pages
printf '\nLeaderboard files pushed to gh-pages. GitHub Pages must finish its build before the update is live.\n'
printf 'Site: https://horizon-llm.github.io/IOI2026/leaderboard/?board=ioi2026subtasks\n'
printf 'Build status: gh api repos/horizon-llm/IOI2026/pages/builds/latest --jq .status\n'
