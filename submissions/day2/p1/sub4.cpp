#include <bits/stdc++.h>
using namespace std;

/* -------------------------------------------------------------
   Teacher procedure
   ------------------------------------------------------------- */
vector<vector<int>> process_step(
    int N, int M, int R,
    vector<int> T,
    vector<vector<int>> A)
{
    // copy current papers – we will modify only allowed ones
    vector<vector<int>> B = A;

    // mark students that raised in this step
    vector<char> raised(N, 0);
    for (int x : T) raised[x] = 1;

    // for every raising student (in the official tests there is at most one)
    for (int s : T) {
        // find a non‑raising student whose paper still has room for a pair
        int best = -1;
        size_t best_len = SIZE_MAX;
        for (int i = 0; i < N; ++i) {
            if (raised[i]) continue;                     // cannot modify
            if (B[i].size() + 2 > 63) continue;         // no space
            if (B[i].size() < best_len) {
                best_len = B[i].size();
                best = i;
            }
        }
        // according to the reasoning a suitable paper always exists
        if (best != -1) {
            B[best].push_back(s);   // student id
            B[best].push_back(R);   // step number
        }
        // if for any reason no paper is suitable we simply do nothing –
        // this situation cannot happen for the given constraints.
    }

    return B;
}

/* -------------------------------------------------------------
   Headmaster procedure
   ------------------------------------------------------------- */
vector<int> determine_steps(
    int N, int M, vector<vector<int>> A)
{
    vector<int> D(N, -1);                // default: never raised
    for (int i = 0; i < N; ++i) {
        const vector<int> &v = A[i];
        for (size_t p = 0; p + 1 < v.size(); p += 2) {
            int student = v[p];
            int step    = v[p + 1];
            if (0 <= student && student < N && 0 <= step && step < M)
                D[student] = step;       // each pair appears once
        }
    }
    return D;
}