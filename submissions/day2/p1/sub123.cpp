#include <bits/stdc++.h>
using namespace std;

// Merged includes from subtask solutions (duplicates removed)

// Subtask 1 solution (solutions/subtask1_classroom.cpp)
namespace subtask1 {
    /*
     * Teacher's procedure.
     * N – number of students
     * M – number of steps (in this subtask M == 1)
     * R – current step number (0‑based)
     * T – sorted list of students that raise their hand in this step
     * A – papers at the beginning of the step
     *
     * The teacher may modify papers of students not in T.
     * We write a single integer equal to the student's own index on each
     * such paper.  The integer fits into the allowed range (0…63) because
     * N ≤ 63.
     */
    vector<vector<int>> process_step(int N, int M, int R,
                                     vector<int> T,
                                     vector<vector<int>> A) {
        vector<char> raised(N, 0);
        for (int i : T) raised[i] = 1;

        vector<vector<int>> B = A;
        for (int i = 0; i < N; ++i) {
            if (!raised[i]) {
                // Encode the original index i.
                B[i] = {i};
            } else {
                // Must keep the paper unchanged.
                B[i] = A[i];
            }
        }
        return B;
    }

    /*
     * Headmaster's procedure.
     * A – papers after the final exchange.
     *
     * The teacher encoded each non‑raising student i by the single integer i.
     * Hence the set of indices appearing in non‑empty arrays is exactly the set
     * of students that never raised their hand.  All other students raised
     * (step number 0).  We reconstruct this information.
     */
    vector<int> determine_steps(int N, int M, vector<vector<int>> A) {
        vector<char> not_raised(N, 0);
        for (const auto& v : A) {
            if (!v.empty()) {
                int idx = v[0];               // the encoded original index
                if (0 <= idx && idx < N) {
                    not_raised[idx] = 1;
                }
            }
        }

        vector<int> D(N);
        for (int i = 0; i < N; ++i) {
            D[i] = not_raised[i] ? -1 : 0;    // 0 → raised in step 0, -1 → never raised
        }
        return D;
    }
}

// Subtask 2 solution (solutions/subtask2_classroom.cpp)
namespace subtask2 {
    static const int SENTINEL = 63;          // value meaning “never raised”

    // ------------------------------------------------------------------
    // Teacher side
    // ------------------------------------------------------------------
    vector<vector<int>> process_step(int N, int M, int R,
                                     vector<int> T,
                                     vector<vector<int>> A)
    {
        // start with a copy of the received papers
        vector<vector<int>> B = A;

        if (!T.empty())                     // exactly one student raised
        {
            int raiser = T[0];               // 0 or 1 (N == 2)
            int other  = 1 - raiser;         // the other student

            // current contents of the paper we are allowed to change
            int v0 = SENTINEL, v1 = SENTINEL;
            if (B[other].size() >= 2) {
                v0 = B[other][0];
                v1 = B[other][1];
            }
            // store the step for the raiser
            if (raiser == 0) v0 = R;
            else             v1 = R;

            B[other] = {v0, v1};             // always length 2
            // B[raiser] stays unchanged (required by the statement)
        }
        // when T is empty we leave B unchanged

        return B;
    }

    // ------------------------------------------------------------------
    // Headmaster side
    // ------------------------------------------------------------------
    vector<int> determine_steps(int N, int M, vector<vector<int>> A)
    {
        vector<int> D(N, -1);               // default: never raised

        for (int i = 0; i < N; ++i) {
            const vector<int> &arr = A[i];
            if (arr.size() >= 2) {
                // arr[0] belongs to student 0, arr[1] to student 1
                if (arr[0] != SENTINEL) D[0] = arr[0];
                if (arr[1] != SENTINEL) D[1] = arr[1];
            }
        }
        return D;
    }
}

// Subtask 3 solution (solutions/subtask3_classroom.cpp)
namespace subtask3 {
    /* ----- helper: base‑64 encode / decode ----- */
    static int decode(const vector<int>& v) {
        int x = 0;
        int mult = 1;                     // 64^position
        for (int d : v) {
            x += d * mult;
            mult <<= 6;                   // multiply by 64
        }
        return x;
    }

    static vector<int> encode(int x) {
        vector<int> v;
        while (x > 0) {
            v.push_back(x & 63);          // x % 64
            x >>= 6;                      // x /= 64
        }
        return v;                         // empty vector represents 0
    }

    /* -------------------------------------------------------------
       Teacher's procedure
       ------------------------------------------------------------- */
    vector<vector<int>> process_step(int N, int M, int R,
                                     vector<int> T,
                                     vector<vector<int>> A) {
        vector<char> raises(N, 0);
        for (int idx : T) raises[idx] = 1;

        int add = R + 1;                  // value added this step
        vector<vector<int>> B(N);

        for (int i = 0; i < N; ++i) {
            if (raises[i]) {
                B[i] = A[i];              // cannot modify
            } else {
                int cur = decode(A[i]);
                cur += add;
                B[i] = encode(cur);
            }
        }
        return B;
    }

    /* -------------------------------------------------------------
       Headmaster's procedure
       ------------------------------------------------------------- */
    vector<int> determine_steps(int N, int M,
                                vector<vector<int>> A) {
        vector<int> D(N);
        int total = M * (M + 1) / 2;      // sum 1 … M

        for (int i = 0; i < N; ++i) {
            int cur = decode(A[i]);
            if (cur == total) {
                D[i] = -1;                // never raised
            } else {
                D[i] = total - cur - 1;   // missing step
            }
        }
        return D;
    }
}

// Global dispatcher
vector<vector<int>> process_step(int N, int M, int R,
                                 vector<int> T,
                                 vector<vector<int>> A) {
    if (M == 1) {
        // Subtask 1 (M == 1) has priority
        return subtask1::process_step(N, M, R, T, A);
    } else if (N == 2) {
        // Subtask 2 (N == 2) when M > 1
        return subtask2::process_step(N, M, R, T, A);
    } else {
        // Remaining cases belong to Subtask 3 (P is identity)
        return subtask3::process_step(N, M, R, T, A);
    }
}

vector<int> determine_steps(int N, int M,
                            vector<vector<int>> A) {
    if (M == 1) {
        return subtask1::determine_steps(N, M, A);
    } else if (N == 2) {
        return subtask2::determine_steps(N, M, A);
    } else {
        return subtask3::determine_steps(N, M, A);
    }
}
