<!--
  Editable page copy for the Arbiter visualiser.

  Each block below is keyed by a `## key` heading that matches a data-md="key"
  attribute in index.html. The loader in index.html fetches this file, splits on
  the `## key` headings, and injects each block's text into the matching element.

  Formatting: **bold**, *italic*, and `code` work. Raw inline HTML is also
  passed through untouched (used for the ARBITER acronym's per-letter bold and
  the coloured "Accepted"), so you can keep or edit those tags directly.

  Edit the prose freely; do NOT rename the `## keys` (they wire to the page).
-->

## title
Arbiter: An Agentic Verifier for Competitive Programming Beyond Score

## teaser-cap
**gpt-oss-120b, refined against Arbiter, clears the IOI 2026 gold-medal line.** Contest score (/600) versus **k**, the candidates drawn per subtask — Arbiter-gated refinement beats plain score@k at every budget, and the gap is widest where samples are scarce. Hover for exact values.

## verifier
A judge can tell you a solution is wrong. It can't tell you *why* — showing the failing input would leak the secret tests that make it trustworthy, and every ranker, majority vote, or LLM-as-judge inherits that same blind spot: a verdict, never a witness. **Arbiter** (<span style="color:var(--ink3)"><b>A</b>gentic <b>R</b>einforcement-learned <b>B</b>ug-finding <b>I</b>nteractive <b>T</b>est <b>E</b>xamine<b>R</b></span>) is a verifier trained to do both — read a problem and one candidate solution, decide Accept or Reject, and on rejection hand back a concrete input that breaks the code, not just a score.

## training
TODO: describe how Arbiter was trained — the base model, the reinforcement-learning setup that rewards finding a valid counterexample, the data it was trained on, and the compute budget.

## results
This page reports an empirical test of what that capability is worth: whether Arbiter-gated refinement, revising against a verifier's counterexample rather than a scalar reward, is by itself sufficient for competitive programming. **gpt-oss-120b** attempts every IOI 2026 subtask and revises against **Arbiter**'s feedback round by round, under a **5-round budget**, until a trajectory is <b style="color:var(--stop)">Accepted</b> and submitted. That refinement loop alone clears IOI 2026's gold-medal score line.

## results-cap
**Results on UOJ-Bench (Hacking) and USACO-Judge**, reported by difficulty split (Easy / Hard) and grouped by inference mode: non-agentic (⚡, single-shot) versus agentic (⚙, tool-using). **Bold** = best in column, <u>underline</u> = runner-up. Arbiter (35B) matches or beats models up to ~45× larger.

## refinement
On IOI 2026 itself, gpt-oss-120b revises each candidate against Arbiter's counterexample under a 5-round budget. Capping refinement at round **R** shows how quickly the verifier's feedback pays off.

## refinement-cap
**One round of refinement does most of the work.** Capping Arbiter-gated refinement at round **R**: score jumps **229 → 347** (clearing gold) from a single counterexample, reaching **376** by R5 — while judge submissions grow only gently (24 → 32), so the gain is bought cheaply.

## artifacts
Every score above is backed by its raw data. Click a subtask to open its fan of 50 trajectories, then any node for the verifier's full turn-by-turn review, or the <img src="figs/coding.png" alt="code icon" style="width:12px;height:12px;vertical-align:-1px"> icon for the program that round submitted and the reasoning that wrote it. The badge beside a problem's score opens its merged final submission.
