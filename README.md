<h1>
  <a href="https://horizon-llm.github.io/IOI2026/"><img src="logo.png" alt="Arbiter" height="38" align="top"></a>
  GPT-OSS-120B + Arbiter Reaches Gold at IOI 2026
</h1>

The International Olympiad in Informatics is one of the international science Olympiads held annually around the world, where exceptional high school students compete on problem analysis, the design of algorithms and data structures, programming and testing. IOI 2026 set [six problems](https://ioi2026.uz/tasks) over two contest days, each worth 100 points. A contestant gets at most **50 submissions per problem** for the whole contest.

For an LLM, generation is cheap and verification is not. It can sample as many candidate programs as compute allows, but only 50 of them per problem will ever be checked. To supply the feedback the judge cannot, we trained **Arbiter**, an agentic verifier. It reads a problem and one candidate program, decides Accept or Reject, and on rejection returns a diagnosis together with a concrete input that breaks the code. The solver then corrects a known bug, so the compute goes into revising one chain deeper instead of widening the pool.

Refining against Arbiter, **[gpt-oss-120b](https://huggingface.co/openai/gpt-oss-120b) scored 385.46 out of 600**, above the 361.12 gold-medal line and equivalent to rank 17. It clears that line on fewer than 100 generations per subtask, where systems that scale test-time compute by search draw thousands. The full write-up is at [horizon-llm.github.io/IOI2026](https://horizon-llm.github.io/IOI2026/).

This repository contains the C++ programs submitted for all six problems, byte for byte, together with a link to each submission on [QOJ](https://qoj.ac), where anyone can read the score off the judge directly.

## Results

| # | Problem | Program | Subtasks | Score | Submission |
|:--|:--|:--|:--|--:|:--|
| **1** | Ball Machine | [`merged.cpp`](submissions/day1/p1/merged.cpp) | S1–S4 | **35.00** | [link](https://qoj.ac/submission/2809712) |
| **2** | Monuments | [`merged.cpp`](submissions/day1/p2/merged.cpp) | S1–S10 | **58.00** | [link](https://qoj.ac/submission/2809714) |
| **3** | Tiling Game | [`merged.cpp`](submissions/day1/p3/merged.cpp) | S1–S5 | **100.00** | [link](https://qoj.ac/submission/2809728) |
| **4** | Classroom Game | [`sub123.cpp`](submissions/day2/p1/sub123.cpp) | S1–S3 | **35.20** | [link](https://qoj.ac/submission/2809772) |
| | | [`sub4.cpp`](submissions/day2/p1/sub4.cpp) | S4 | | [link](https://qoj.ac/submission/2810116) |
| | | [`sub5.cpp`](submissions/day2/p1/sub5.cpp) | S5 | | [link](https://qoj.ac/submission/2810256) |
| **5** | Magic City | [`merged.cpp`](submissions/day2/p2/merged.cpp) | S1–S50 † | **100.00** | [link](https://qoj.ac/submission/2810479) |
| **6** | Partition | [`sub1456789.cpp`](submissions/day2/p3/sub1456789.cpp) | S1, S4–S9 | **62.00** | [link](https://qoj.ac/submission/2810568) |
| | | [`sub23.cpp`](submissions/day2/p3/sub23.cpp) | S2–S3 | | [link](https://qoj.ac/submission/2810611) |
| | | | | **390.20 / 600** | |

<small>† Magic City ships **50** subtasks. The last of them are narrow variations on one another, so we grouped them for refinement and treat the task as eight: subtasks 1–5 stand alone, 6–10 become S6 (20 points), 11–12 become S7 (6 points), and 13–50 become S8 (38 points). The grouping is ours and the judge knows nothing about it: the [submission](https://qoj.ac/submission/2810479) is scored on all 50, and takes every one of them.</small>

Problems 1, 2, 3 and 5 were each answered by one program, so its score on QOJ is the problem score. Problems 4 and 6 were each answered by several: each subtask is solved separately and then merged into one file behind a dispatcher, and on these two the merge is impossible — nothing the grader exposes at run time tells their subtasks apart. They ship as three and two files, and the 50 submissions have to be split among them, which makes the score a random variable: **385.46** is its expectation, the **390.20** above is what these submissions happened to earn. The section below shows how they add up. Every trajectory is on the [write-up page](https://horizon-llm.github.io/IOI2026/).

## Verifying a score

IOI scores a problem by taking, for each subtask, the best result any submission achieved on that subtask, and summing across subtasks. For the four single-program problems that reduces to the number QOJ prints. For the other two it is a per-subtask maximum over the programs, so a subtask worth 25 that one program left at 0 and another took to 13.75 contributes 13.75:

**Problem 4 · Classroom Game**

| Submission | S1 (4) | S2 (6) | S3 (9) | S4 (25) | S5 (56) | Total |
|:--|--:|--:|--:|--:|--:|--:|
| `sub123` [link](https://qoj.ac/submission/2809772) | 4 | 6 | 9 | 0 | 0 | 19.00 |
| `sub4` [link](https://qoj.ac/submission/2810116) | 0 | 3.3 | 0 | 13.75 | 0 | 17.05 |
| `sub5` [link](https://qoj.ac/submission/2810256) | 0.1752 | 6 | 0.3942 | 1.095 | 2.4528 | 10.12 |
| **best per subtask** | **4** | **6** | **9** | **13.75** | **2.4528** | **35.20** |

**Problem 6 · Partition**

| Submission | S1 (5) | S2 (4) | S3 (7) | S4 (10) | S5 (7) | S6 (12) | S7 (17) | S8 (20) | S9 (18) | Total |
|:--|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| `sub1456789` [link](https://qoj.ac/submission/2810568) | 5 | 0 | 0 | 10 | 7 | 12 | 17 | 0 | 0 | 51.00 |
| `sub23` [link](https://qoj.ac/submission/2810611) | 0 | 4 | 7 | 0 | 0 | 0 | 0 | 0 | 0 | 11.00 |
| **best per subtask** | **5** | **4** | **7** | **10** | **7** | **12** | **17** | **0** | **0** | **62.00** |

## Acknowledgements

We thank **[QOJ](https://qoj.ac)** for mirroring the IOI 2026 problems and hosting the submissions linked above, and **[UOJ](https://uoj.ac)** for open-sourcing the [judge](https://github.com/UniversalOJ/UOJ-System) we built our training environment on.
