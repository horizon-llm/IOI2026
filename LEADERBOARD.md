# IOI 2026 leaderboard

The `leaderboard` branch maintains the static leaderboard in `leaderboard/`. It is designed to live at [horizon-llm.github.io/IOI2026/leaderboard/](https://horizon-llm.github.io/IOI2026/leaderboard/?board=ioi2026subtasks). The existing Arbiter write-up continues to live at the site root on `gh-pages`.

The UI is dedicated to IOI 2026 subtasks. Rows use plain model names; evaluation methods are shown in the detail panel. Columns with no published measurements are hidden.

Model runs can share a display name while differing in configuration, token limit, or sample count. Their stable run IDs keep those results separate. Short configuration labels appear below the model name.

For a reference snapshot approved for publication, `node scripts/import-reference.mjs <snapshot.json>` imports score rows into `leaderboard/data.json`, excluding `longest_*`, `ojblind_*`, and `smoke_*` placeholders. It preserves the existing Arbiter result, validates score totals and subtask ordering, and omits internal submission IDs and links. Tooltip precision is preserved; rounded token displays are not treated as exact measurements. The browser continues to load only the published JSON file.

## Preview and checks

No package installation or build step is required. From the repository root:

```sh
python3 -m http.server 4173
```

Open [localhost:4173/leaderboard/?board=ioi2026subtasks](http://localhost:4173/leaderboard/?board=ioi2026subtasks). Serve over HTTP rather than opening the HTML file directly, because the page loads `data.json`.

Run the data and scoring checks with Node.js:

```sh
node --test tests/*.test.mjs
```

## Updating results

Edit `leaderboard/data.json`, validate it, review the page, and commit the update on `leaderboard`. The browser automatically rereads this same-origin file every 15 seconds while the page is visible. A refresh checks for a newly published snapshot; it does not connect to the internal judge.

The current snapshot includes all **24 model runs** from the supplied IOI 2026 subtask board, approved for publication, plus the existing **GPT-OSS-120B with Arbiter** result. The 19 `longest_*`, `ojblind_*`, and `smoke_*` placeholder/test rows are excluded. Different context limits, sample counts, and reasoning or temperature settings remain separate runs, with concise model names.

For imported sampling runs, `score` is the source **Score@k**, and `scoreAt1` is the source **Score@1**. The Arbiter entry instead reports an observed QOJ total of **390.2028 / 600** (390.20 rounded) and an expected contest score of **385.46 / 600**. Its `k` and `scoreAt1` remain unknown; this evaluation must not be relabeled as a sampled run. `scoreKind` identifies this distinction in the JSON, detail panel, and CSV.

The version 1 data contract is:

| Field | Meaning |
| --- | --- |
| `schemaVersion` | Currently `1`. |
| `board` | `ioi2026subtasks`. |
| `title` | Display title. |
| `updatedAt` | Source snapshot timestamp in ISO 8601 format, or `null` if not known. Do not replace it merely because the browser refreshed. |
| `source` | Source `label` and public `url`. Prefer a commit-pinned link. |
| `problems` | Ordered definitions with stable `id`, `label`, `name`, contest `day`, `maxScore`, and ordered subtask `weights`. |
| `entries` | Result rows, each with a stable `id`, `name`, `description`, `status`, and the fields below. |

Each entry contains the total `score`, optional `expectedScore`, `contextTokens`, `k`, `scoreAt1`, `passAtK`, `passAt1`, `avgThinkTokens`, and a `sourceNote` explaining the evaluation. Imported entries also include the original `sourceRun` identifier, a short `configuration`, and `scoreKind: "sampled"`; the Arbiter entry uses `scoreKind: "observed"`. Its `problems` array matches the definitions by `id`; each result has `score`, optional `average`, ordered `subtasks` with `score` and `average`, and public `submissions` links with `label` and `url` where available. Imported rows omit internal submission links and IDs.

Use `null` for an unknown metric or subtask result. Zero is a known zero score. Do not fill missing pass rates, averages, token budgets, or sample counts from unrelated experiments. The Arbiter entry has known P1 and P2 totals but no imported subtask breakdown. All 24 sampling runs have their 41 subtask best scores and averages from the reference board. Tooltip averages are rounded at the source, so their sums may differ slightly from the reported overall Score@1. Token counts likewise retain the source display precision (K = 1,000).

There are six problems worth 100 points each and 41 displayed subtask groups. Magic City has 50 official subtasks grouped into eight for this view: S1–S5 individually, S6–S10 together, S11–S12 together, and S13–S50 together. The grouped weights are presentation metadata, not a change to the official judge.

## Publishing

GitHub Pages remains configured to publish `/` from `gh-pages`. The publisher copies only the committed `leaderboard/` subtree from the current `leaderboard` commit into a temporary clone of `gh-pages`, then makes a normal fast-forward push. It preserves the write-up, its assets, and the Pages settings. It never force-pushes. If `gh-pages` advances during publication, rerun the publisher against the new head.

Requirements: Bash, Git, Node.js, `tar`, `rsync`, an existing Git commit identity, and GitHub push access. Commit and push the source branch first:

```sh
git push -u origin leaderboard
bash scripts/publish-leaderboard.sh --dry-run
bash scripts/publish-leaderboard.sh --publish
```

With no argument the script performs a dry run. It runs the checks, stages the proposed subtree update in a temporary clone, and prints the change summary without committing or pushing. Uncommitted files in `leaderboard/` cause it to stop, so deployment always matches a source commit.

After publication, verify the Pages build and the page:

```sh
gh api repos/horizon-llm/IOI2026/pages/builds/latest --jq '{status,commit,error}'
```

An ordinary authenticated push triggers the existing legacy Pages deployment. No automatic publication workflow is installed: GitHub documents that commits pushed with a workflow's `GITHUB_TOKEN` do not trigger a legacy Pages build. See [GitHub's publishing-source documentation](https://docs.github.com/en/pages/getting-started-with-github-pages/configuring-a-publishing-source-for-your-github-pages-site#troubleshooting-publishing-from-a-branch). Publishing from a local authenticated Git session avoids changing the repository's current hosting configuration.
