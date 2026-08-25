#include <bits/stdc++.h>
using namespace std;

// includes from solution files removed (they were #include <bits/stdc++.h> and 'using namespace std;').

// solutions/subtask2_partition.cpp
namespace subtask2 {
    /* Ulug'bek's procedure.
       Returns a list of numbers (all equal to 1) to be added.
       At most K-1 numbers are returned. */
    vector<int> add_numbers(vector<int> A, int K, int M) {
        // M is always 1 in this subtask, so only the value 1 can be added.
        int N = static_cast<int>(A.size());
        int remainder = N % K;
        int S = (K - remainder) % K;          // 0 <= S <= K-1, makes N+S divisible by K
        return vector<int>(S, 1);             // S copies of 1
    }

    /* Temur's procedure.
       B is the sorted final array (all ones). Returns a group assignment
       for each position, groups are numbered 0 .. K-1. */
    vector<int> find_partition(vector<int> B, int K) {
        int total = static_cast<int>(B.size());   // total = N + S, guaranteed divisible by K
        int groupSize = total / K;                 // each group must contain this many elements
        vector<int> P(total);
        int idx = 0;
        for (int g = 0; g < K; ++g) {
            for (int i = 0; i < groupSize; ++i) {
                P[idx++] = g;
            }
        }
        return P;
    }
}

// solutions/subtask3_partition.cpp
namespace subtask3 {
    /* -------------------------------------------------------------
       Ulug'bek – add numbers
       ------------------------------------------------------------- */
    vector<int> add_numbers(vector<int> A, int K, int M) {
        long long sum = 0;
        int c1 = 0;                     // original number of 1's
        for (int x : A) {
            sum += x;
            if (x == 1) ++c1;
        }

        /* -------- M = 1 ------------------------------------------------ */
        if (M == 1) {
            int need = (K - (int)(sum % K)) % K;      // 0 … K-1
            return vector<int>(need, 1);
        }

        /* -------- M = 2 ------------------------------------------------ */
        int maxAddElements = K - 1;                 // at most this many numbers
        int maxAddSum = 2 * maxAddElements;         // 2·K‑2, maximal attainable sum

        for (int X = 0; X <= maxAddSum; ++X) {      // X = total added sum
            if ((sum + X) % K != 0) continue;       // must become divisible

            long long t = (sum + X) / K;
            int r = (int)(t % 2);                  // parity of group sum

            int a1_min = X % 2;                     // smallest possible #1's
            int a1_max = min(X, maxAddSum - X);     // largest possible #1's
            if (a1_min > a1_max) continue;         // impossible (should not happen)

            if (r == 0) {                           // even group sum – ok
                int a1 = a1_min;                    // any feasible value works
                int a2 = (X - a1) / 2;
                vector<int> C;
                C.reserve(a1 + a2);
                C.insert(C.end(), a1, 1);
                C.insert(C.end(), a2, 2);
                return C;
            } else {                               // odd group sum, need enough 1's
                int needOnes = K - c1;              // we want c1 + a1 ≥ K
                if (needOnes < 0) needOnes = 0;

                int a1 = max(needOnes, a1_min);
                if ((a1 % 2) != (X % 2)) ++a1;      // keep the parity of X
                if (a1 <= a1_max) {
                    int a2 = (X - a1) / 2;
                    vector<int> C;
                    C.reserve(a1 + a2);
                    C.insert(C.end(), a1, 1);
                    C.insert(C.end(), a2, 2);
                    return C;
                }
            }
        }

        // Theoretically unreachable – a fallback that obeys the limits.
        return vector<int>(K - 1, 1);
    }

    /* -------------------------------------------------------------
       Temur – find a partition
       ------------------------------------------------------------- */
    vector<int> find_partition(vector<int> B, int K) {
        int n = (int)B.size();
        long long total = 0;
        int ones = 0, twos = 0;
        for (int x : B) {
            total += x;
            if (x == 1) ++ones;
            else ++twos;
        }

        long long t = total / K;                // sum of each group

        /* ---- distribute the twos ------------------------------------- */
        vector<int> need_twos(K, 0);
        int base = twos / K;
        int rem  = twos % K;
        for (int i = 0; i < K; ++i)
            need_twos[i] = base + (i < rem ? 1 : 0);

        /* ---- how many ones each group still needs -------------------- */
        vector<int> need_ones(K, 0);
        for (int i = 0; i < K; ++i)
            need_ones[i] = (int)(t - 2LL * need_twos[i]);   // non‑negative

        /* ---- positions of 1's and 2's -------------------------------- */
        vector<int> idx_one, idx_two;
        idx_one.reserve(ones);
        idx_two.reserve(twos);
        for (int i = 0; i < n; ++i) {
            if (B[i] == 1) idx_one.push_back(i);
            else           idx_two.push_back(i);
        }

        vector<int> P(n, -1);
        int p1 = 0, p2 = 0;
        for (int g = 0; g < K; ++g) {
            for (int cnt = 0; cnt < need_ones[g]; ++cnt)
                P[idx_one[p1++]] = g;
            for (int cnt = 0; cnt < need_twos[g]; ++cnt)
                P[idx_two[p2++]] = g;
        }
        return P;
    }
}

// Dispatcher

vector<int> add_numbers(vector<int> A, int K, int M) {
    // Use the general solution (subtask3) which works for both M=1 and M=2.
    // Subtask2's version is not needed because the subtask3 implementation
    // already handles the M = 1 case.
    return subtask3::add_numbers(A, K, M);
}

vector<int> find_partition(vector<int> B, int K) {
    // The find_partition routine cannot distinguish which subtask is being
    // solved (M is not available). The subtask3 implementation works for
    // both M = 1 and M = 2, so we always use it.
    // Hence the subtask2 version is unreachable.
    return subtask3::find_partition(B, K);
}
