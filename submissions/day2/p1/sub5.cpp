#include <bits/stdc++.h>
using namespace std;

static const int UNKNOWN = 63;          // sentinel for “never raised”

// -------------------------------------------------------------------
// Teacher
// -------------------------------------------------------------------
vector<vector<int>> process_step(int N, int M, int R,
                                 vector<int> T,
                                 vector<vector<int>> A)
{
    // who raises now?
    vector<char> isRaiser(N, 0);
    for (int x : T) if (0 <= x && x < N) isRaiser[x] = 1;

    // --------------------------------------------------------------
    // 1. reconstruct the knowledge already written on the papers
    vector<int> G(N, UNKNOWN);
    for (int s = 0; s < N; ++s) {
        if ((int)A[s].size() == N) {                 // a full vector was written earlier
            const vector<int>& arr = A[s];
            for (int i = 0; i < N; ++i) {
                int v = arr[i];
                if (v != UNKNOWN) G[i] = v;         // keep the real step, ignore sentinel
            }
        }
    }

    // --------------------------------------------------------------
    // 2. add the information of the current step
    for (int x : T) G[x] = R;                        // step of every raiser

    // --------------------------------------------------------------
    // 3. write the (updated) knowledge onto all papers that may be changed
    vector<vector<int>> B(N);
    for (int i = 0; i < N; ++i) {
        if (isRaiser[i]) {
            B[i] = A[i];                // untouched
        } else {
            B[i] = G;                   // copy of the whole vector
        }
    }
    return B;
}

// -------------------------------------------------------------------
// Head‑master
// -------------------------------------------------------------------
vector<int> determine_steps(int N, int M,
                            vector<vector<int>> A)
{
    vector<int> G(N, UNKNOWN);
    for (int s = 0; s < N; ++s) {
        if ((int)A[s].size() == N) {
            const vector<int>& arr = A[s];
            for (int i = 0; i < N; ++i) {
                int v = arr[i];
                if (v != UNKNOWN) G[i] = v;
            }
        }
    }

    vector<int> D(N);
    for (int i = 0; i < N; ++i) {
        D[i] = (G[i] == UNKNOWN) ? -1 : G[i];
    }
    return D;
}