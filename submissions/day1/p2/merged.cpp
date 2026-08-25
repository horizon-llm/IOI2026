/*  --------------------------------------------------------------------------
    Merged solution for the “Monuments” problem.
    The program contains the original solutions for each subtask, each placed
    in its own namespace to avoid name clashes.  The dispatcher at the end
    selects the appropriate sub‑solution based on the properties of the input.
    -------------------------------------------------------------------------- */
#include <bits/stdc++.h>
using namespace std;

/*==========================================================================*/
/* solutions/subtask1_monuments.cpp                                        */
/*==========================================================================*/
namespace Subtask1 {
#include <bits/stdc++.h>
using namespace std;

long long get_cost(std::vector<int> X, std::vector<int> P) {
    // In this subtask all monuments are ancient (M = N), therefore they cannot be moved.
    // The configuration must already be symmetric, otherwise it is impossible.
    unordered_map<long long, int> cnt;
    cnt.reserve(X.size() * 2);
    for (int v : X) cnt[v]++;

    for (auto const &kv : cnt) {
        long long x = kv.first;
        if (x == 0) continue;                     // coordinate 0 has no restriction
        int c1 = kv.second;
        int c2 = 0;
        auto it = cnt.find(-x);
        if (it != cnt.end()) c2 = it->second;
        if (c1 != c2) return -1;                  // imbalance -> impossible
    }
    // Symmetric already, no moves can be performed, total cost is 0.
    return 0;
}
}

/*==========================================================================*/
/* solutions/subtask2_monuments.cpp                                        */
/*==========================================================================*/
namespace Subtask2 {
#include <bits/stdc++.h>
using namespace std;

/*  Subtask 2 : M = 0  (no ancient monuments)  */
long long get_cost(std::vector<int> X, std::vector<int> P) {
    // P is empty – it is ignored.
    long long total = 0;                     // Σ |X[i]|
    std::vector<long long> neg, pos;              // |x| for negatives / positives

    for (int xi : X) {
        long long x = xi;
        long long ax = (x >= 0) ? x : -x;    // absolute value
        total += ax;
        if (x < 0)   neg.push_back(ax);     // store |x|
        else if (x > 0) pos.push_back(ax);  // store  x
        // zeros are already at the required place, nothing to store
    }

    sort(neg.begin(), neg.end(), greater<long long>());
    sort(pos.begin(), pos.end(), greater<long long>());

    size_t k = min(neg.size(), pos.size());
    long long saved = 0;                     // Σ min(neg[i], pos[i])
    for (size_t i = 0; i < k; ++i) {
        saved += min(neg[i], pos[i]);
    }

    long long answer = total - 2 * saved;    // minimal total cost
    return answer;                           // always possible, never -1
}
}

/*==========================================================================*/
/* solutions/subtask3_monuments.cpp                                        */
/*==========================================================================*/
namespace Subtask3 {
#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct NegItem {
    int64 val;   // absolute value
    bool anc;    // true if this negative is an ancient
};

long long get_cost(std::vector<int> X, std::vector<int> P) {
    int N = (int)X.size();
    int M = (int)P.size();

    /* not enough movable monuments -> impossible */
    if ((int64)N < 2LL * M) return -1;

    /* mark ancients */
    vector<char> isAnc(N, 0);
    for (int idx : P) isAnc[idx] = 1;

    int64 sumAbs = 0;
    vector<int64> pos;                 // > 0
    vector<NegItem> neg;               // < 0, with flag

    for (int i = 0; i < N; ++i) {
        int64 x = X[i];
        sumAbs += llabs(x);
        if (x > 0) pos.push_back(x);
        else if (x < 0) neg.push_back({-x, (bool)isAnc[i]});
        /* x == 0 -> nothing to store, cost 0 */
    }

    int POS = (int)pos.size();
    int NEG = (int)neg.size();

    sort(pos.begin(), pos.end(), greater<int64>());
    sort(neg.begin(), neg.end(),
         [](const NegItem& a, const NegItem& b) { return a.val > b.val; });

    int Q = min(POS, NEG);                 // number of negative‑positive pairs
    int64 R = (int64)N - 2LL * Q;          // leftover points

    // lower bounds for ancients that must be paired with a positive
    int64 L1 = max<int64>(0LL, 2LL * M - (int64)N + (int64)POS);
    int64 L2 = (int64)M - R / 2;           // floor(R/2)
    if (L2 < 0) L2 = 0;
    int64 needAnc = max(L1, L2);           // required ancients inside the Q selection

    // safety – with N >= 2M this never happens, but keep it for completeness
    if (needAnc > Q) return -1;

    // ----- initial selection : first Q negatives -----
    vector<int64> selAnc, selNonAnc;    // values only
    int ancSel = 0;
    for (int i = 0; i < Q; ++i) {
        if (neg[i].anc) {
            ++ancSel;
            selAnc.push_back(neg[i].val);
        } else {
            selNonAnc.push_back(neg[i].val);
        }
    }

    // ----- if too few ancients, replace some non‑ancients -----
    if (ancSel < needAnc) {
        int64 delta = needAnc - ancSel;               // how many ancients we need to add
        // collect the largest remaining ancients (they are after position Q)
        vector<int64> ancRemain;
        ancRemain.reserve(delta);
        for (int i = Q; i < NEG && (int)ancRemain.size() < delta; ++i) {
            if (neg[i].anc) ancRemain.push_back(neg[i].val);
        }
        // remove the smallest 'delta' non‑ancients from the current selection
        sort(selNonAnc.begin(), selNonAnc.end());      // ascending
        selNonAnc.erase(selNonAnc.begin(),
                         selNonAnc.begin() + (size_t)delta);
        // add the ancients we took
        for (int64 v : ancRemain) selAnc.push_back(v);
        // now ancSel == needAnc
    }

    // ----- final list of selected negatives -----
    vector<int64> selNeg;
    selNeg.reserve(Q);
    for (int64 v : selAnc)  selNeg.push_back(v);
    for (int64 v : selNonAnc) selNeg.push_back(v);
    sort(selNeg.begin(), selNeg.end(), greater<int64>());   // descending

    // ----- compute total benefit -----
    int64 benefit = 0;
    for (int i = 0; i < Q; ++i) {
        int64 mn = selNeg[i];
        int64 pv = pos[i];
        benefit += (mn < pv ? mn : pv);
    }

    int64 answer = sumAbs - 2LL * benefit;
    return answer;
}
}

/*==========================================================================*/
/* solutions/subtask4_monuments.cpp                                        */
/*==========================================================================*/
namespace Subtask4 {
#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = (1LL << 60);

/*---------------------------------------------------------------*/

long long get_cost(vector<int> X, vector<int> P) {
    int N = (int)X.size();
    int M = (int)P.size();

    /* -----------------------------------------------------------
       split monuments into immutable and movable
       ----------------------------------------------------------- */
    vector<char> isAnc(N, 0);
    for (int idx : P) isAnc[idx] = 1;

    vector<int64> movable;                 // positions of the movable ones
    for (int i = 0; i < N; ++i)
        if (!isAnc[i]) movable.push_back(X[i]);
    int K = (int)movable.size();           // number of movable monuments

    /* -----------------------------------------------------------
       count ancient monuments per coordinate
       ----------------------------------------------------------- */
    unordered_map<int64,int> ancCnt;
    for (int idx : P) ancCnt[(int64)X[idx]]++;

    /* -----------------------------------------------------------
       build the list of mandatory slots
       ----------------------------------------------------------- */
    vector<int64> mandatory;               // one entry per forced movable
    unordered_set<int64> absVals;
    for (auto &kv : ancCnt) {
        int64 c = kv.first;
        if (c != 0) absVals.insert(llabs(c));
    }
    for (int64 x : absVals) {
        int pos = ancCnt.count(x) ? ancCnt[x] : 0;
        int neg = ancCnt.count(-x) ? ancCnt[-x] : 0;
        int diff = pos - neg;              // d(x)
        if (diff > 0) {
            for (int i = 0; i < diff; ++i) mandatory.push_back(-x);
        } else if (diff < 0) {
            for (int i = 0; i < -diff; ++i) mandatory.push_back(x);
        }
    }
    int D = (int)mandatory.size();

    if (D > K) return -1;                  // impossible (Lemma 3)

    /* -----------------------------------------------------------
       DP for the leftovers (pair / zero) – section 6
       ----------------------------------------------------------- */
    int fullMask = (1 << K) - 1;
    vector<int64> dp(1 << K, INF);
    dp[0] = 0;
    for (int mask = 1; mask <= fullMask; ++mask) {
        int i = __builtin_ctz(mask);                 // lowest set bit
        int64 best = INF;

        // put i on 0
        best = min(best, llabs(movable[i]) + dp[mask ^ (1 << i)]);

        // pair i with another j
        int rest = mask ^ (1 << i);
        for (int sub = rest; sub; sub &= sub - 1) {
            int j = __builtin_ctz(sub);
            int64 cand = llabs(movable[i] + movable[j]) +
                         dp[rest ^ (1 << j)];
            if (cand < best) best = cand;
        }
        dp[mask] = best;
    }

    /* -----------------------------------------------------------
       cost matrix for mandatory slots
       ----------------------------------------------------------- */
    vector<vector<int64>> costMat(K, vector<int64>(D, 0));
    for (int i = 0; i < K; ++i)
        for (int j = 0; j < D; ++j)
            costMat[i][j] = llabs(movable[i] - mandatory[j]);

    /* -----------------------------------------------------------
       enumerate all subsets of size D, solve assignment and combine
       ----------------------------------------------------------- */
    int64 answer = INF;
    int limit = 1 << K;
    for (int mask = 0; mask < limit; ++mask) {
        if (__builtin_popcount(mask) != D) continue;

        // collect indices of the chosen movable monuments
        vector<int> items;
        items.reserve(D);
        for (int i = 0; i < K; ++i)
            if (mask & (1 << i)) items.push_back(i);

        // ----- assignment of these items to the mandatory slots -----
        int64 assignCost = 0;
        if (D > 0) {
            int SZ = 1 << D;
            vector<int64> assignDP(SZ, INF);
            assignDP[0] = 0;
            for (int cur = 0; cur < SZ; ++cur) {
                int pos = __builtin_popcount(cur);
                if (pos >= D) continue;
                int64 curVal = assignDP[cur];
                if (curVal == INF) continue;
                for (int s = 0; s < D; ++s) {
                    if (cur & (1 << s)) continue;
                    int nxt = cur | (1 << s);
                    int64 cand = curVal + costMat[items[pos]][s];
                    if (cand < assignDP[nxt]) assignDP[nxt] = cand;
                }
            }
            assignCost = assignDP[SZ - 1];
        }

        int restMask = fullMask ^ mask;
        int64 total = assignCost + dp[restMask];
        if (total < answer) answer = total;
    }

    return answer;
}
}

/*==========================================================================*/
/* solutions/subtask5_monuments.cpp                                        */
/*==========================================================================*/
namespace Subtask5 {
#include <bits/stdc++.h>
using namespace std;

long long get_cost(vector<int> X, vector<int> P) {
    int N = (int)X.size();
    int M = (int)P.size();

    vector<char> isAnc(N, false);
    for (int idx : P) isAnc[idx] = true;

    // movable monuments
    vector<long long> movable;
    movable.reserve(N);
    // counts of ancient monuments by absolute value
    unordered_map<long long, pair<int,int>> cnt; // d -> (pos,neg)
    long long ancZero = 0;
    for (int i = 0; i < N; ++i) {
        long long c = X[i];
        if (isAnc[i]) {
            if (c > 0) cnt[c].first++;
            else if (c < 0) cnt[-c].second++;
            else ancZero++;
        } else {
            movable.push_back(c);
        }
    }

    // mandatory placements to balance the ancient monuments
    vector<long long> mandatory;
    mandatory.reserve(N);
    for (auto &e : cnt) {
        long long d = e.first;
        int pos = e.second.first;
        int neg = e.second.second;
        if (pos > neg) {
            int need = pos - neg;
            for (int k = 0; k < need; ++k) mandatory.push_back(-d);
        } else if (neg > pos) {
            int need = neg - pos;
            for (int k = 0; k < need; ++k) mandatory.push_back(d);
        }
    }
    sort(mandatory.begin(), mandatory.end());

    int K = (int)movable.size();          // number of movable monuments
    int R = (int)mandatory.size();        // number of forced positions

    if (K < R) return -1;                 // not enough movable monuments

    /* ----------- DP for leftover monuments (pairing / zero) ----------- */
    vector<long long> zeroCost(K);
    for (int i = 0; i < K; ++i) zeroCost[i] = llabs(movable[i]);

    vector<vector<long long>> pairCost(K, vector<long long>(K, 0));
    for (int i = 0; i < K; ++i) {
        for (int j = i + 1; j < K; ++j) {
            pairCost[i][j] = llabs(movable[i] + movable[j]);
            pairCost[j][i] = pairCost[i][j];
        }
    }

    int totalMask = 1 << K;
    const long long INF = (1LL << 60);
    vector<long long> dp(totalMask, INF);
    dp[0] = 0;
    for (int mask = 1; mask < totalMask; ++mask) {
        int i = __builtin_ctz(mask);               // first set bit
        int maskWithoutI = mask ^ (1 << i);
        long long best = dp[maskWithoutI] + zeroCost[i];   // place i at 0
        int rest = maskWithoutI;
        while (rest) {
            int j = __builtin_ctz(rest);
            int newMask = maskWithoutI ^ (1 << j);
            long long cand = dp[newMask] + pairCost[i][j];
            if (cand < best) best = cand;
            rest ^= (1 << j);
        }
        dp[mask] = best;
    }

    /* --------------- Choose which monuments fill mandatory --------------- */
    long long answer = INF;
    int fullMask = totalMask - 1;

    if (R == 0) {
        answer = dp[fullMask];
    } else {
        for (int mask = 0; mask < totalMask; ++mask) {
            if (__builtin_popcount(mask) != R) continue;
            // collect the chosen movable coordinates
            vector<long long> chosen;
            chosen.reserve(R);
            for (int i = 0; i < K; ++i)
                if (mask & (1 << i)) chosen.push_back(movable[i]);
            sort(chosen.begin(), chosen.end());

            long long costMand = 0;
            for (int i = 0; i < R; ++i)
                costMand += llabs(chosen[i] - mandatory[i]);

            int leftMask = fullMask ^ mask;
            long long total = costMand + dp[leftMask];
            if (total < answer) answer = total;
        }
    }
    return answer;
}
}

/*==========================================================================*/
/* solutions/subtask6_monuments.cpp                                        */
/*==========================================================================*/
namespace Subtask6 {
#include <bits/stdc++.h>
using namespace std;

class MonumentsSolver {
public:
    long long get_cost(vector<int> X, vector<int> P) {
        const int N = (int)X.size();
        const int M = (int)P.size();

        /* -------------------------------------------------------------
           1.  counts of ancient monuments on each side
        ------------------------------------------------------------- */
        unordered_map<long long,int> cntPos, cntNeg;   // by absolute distance
        vector<char> isAnc(N, 0);
        for (int idx : P) isAnc[idx] = 1;

        for (int i = 0; i < N; ++i) if (isAnc[i]) {
            long long x = X[i];
            if (x > 0) cntPos[ llabs(x) ]++;
            else if (x < 0) cntNeg[ llabs(x) ]++;
            // x == 0 does not influence symmetry
        }

        /* -------------------------------------------------------------
           2.  forced positions L
        ------------------------------------------------------------- */
        vector<long long> forced;          // each element is a coordinate
        for (auto &kv : cntPos) {
            long long d = kv.first;
            int a = cntNeg.count(d) ? cntNeg[d] : 0;
            int b = kv.second;
            int diff = b - a;
            if (diff > 0) forced.insert(forced.end(), diff, -d);   // need -d
            else if (diff < 0) forced.insert(forced.end(), -diff, d); // need +d
        }
        for (auto &kv : cntNeg) {
            long long d = kv.first;
            if (cntPos.count(d)) continue;          // already processed
            int diff = -kv.second;                   // a = cntNeg[d], b = 0
            forced.insert(forced.end(), -diff, d);
        }

        int S = (int)forced.size();          // number of forced positions

        /* -------------------------------------------------------------
           3.  movable monuments
        ------------------------------------------------------------- */
        struct Node {
            long long x;          // original coordinate
            long long mag;        // |x|
            int sign;             // -1,0,+1
        };
        vector<Node> movable;
        movable.reserve(N);
        long long sumAbs = 0;
        for (int i = 0; i < N; ++i) if (!isAnc[i]) {
            long long x = X[i];
            long long m = llabs(x);
            sumAbs += m;
            int s = (x > 0) ? 1 : (x < 0 ? -1 : 0);
            movable.push_back({x, m, s});
        }
        int K = (int)movable.size();

        if (S > K) return -1;               // impossible

        /* -------------------------------------------------------------
           4.  sort movable by decreasing absolute value
        ------------------------------------------------------------- */
        sort(movable.begin(), movable.end(),
             [](const Node& a, const Node& b){ return a.mag > b.mag; });

        /* -------------------------------------------------------------
           5.  pre‑compute reduction to each forced coordinate
        ------------------------------------------------------------- */
        vector<vector<long long>> red(K, vector<long long>(S));
        for (int i = 0; i < K; ++i) {
            for (int j = 0; j < S; ++j) {
                red[i][j] = movable[i].mag - llabs(movable[i].x - forced[j]);
            }
        }

        /* -------------------------------------------------------------
           6.  DP over (mask of used forced positions, diff)
        ------------------------------------------------------------- */
        int maskCnt = 1 << S;
        int diffOffset = K;                     // to make index non‑negative
        int diffSize   = 2 * K + 1;
        const long long NEG = -(1LL<<60);

        vector<long long> dp(maskCnt * diffSize, NEG);
        dp[0 * diffSize + diffOffset] = 0;      // mask=0, diff=0

        for (int i = 0; i < K; ++i) {
            const Node &cur = movable[i];
            vector<long long> nxt(maskCnt * diffSize, NEG);
            int freeMaskAll = maskCnt - 1;      // bits for all forced positions

            for (int mask = 0; mask < maskCnt; ++mask) {
                int freeMask = freeMaskAll ^ mask;   // positions still free
                int base = mask * diffSize;
                for (int dIdx = 0; dIdx < diffSize; ++dIdx) {
                    long long curVal = dp[base + dIdx];
                    if (curVal == NEG) continue;
                    int diff = dIdx - diffOffset;

                    /* ---- leave the monument free (greedy pairing) ---- */
                    int newDiff = diff;
                    long long add = 0;
                    if (cur.sign > 0) {                 // positive
                        if (diff < 0) {                 // there is a negative surplus
                            add = 2LL * cur.mag;
                            newDiff = diff + 1;
                        } else {
                            newDiff = diff + 1;
                        }
                    } else if (cur.sign < 0) {          // negative
                        if (diff > 0) {                 // there is a positive surplus
                            add = 2LL * cur.mag;
                            newDiff = diff - 1;
                        } else {
                            newDiff = diff - 1;
                        }
                    } else {                            // zero
                        newDiff = diff;
                        add = 0;
                    }
                    int newIdx = base + (newDiff + diffOffset);
                    long long cand = curVal + add;
                    if (cand > nxt[newIdx]) nxt[newIdx] = cand;

                    /* ---- use the monument for a forced position ---- */
                    if (freeMask) {
                        int bits = freeMask;
                        while (bits) {
                            int j = __builtin_ctz(bits);
                            bits &= bits - 1;
                            int newMask = mask | (1 << j);
                            int idx = newMask * diffSize + dIdx;
                            long long cand2 = curVal + red[i][j];
                            if (cand2 > nxt[idx]) nxt[idx] = cand2;
                        }
                    }
                }
            }
            dp.swap(nxt);
        }

        /* -------------------------------------------------------------
           7.  retrieve best reduction, compute answer
        ------------------------------------------------------------- */
        int fullMask = maskCnt - 1;
        long long best = NEG;
        int baseFull = fullMask * diffSize;
        for (int dIdx = 0; dIdx < diffSize; ++dIdx)
            best = max(best, dp[baseFull + dIdx]);

        // best is always reachable because S ≤ K
        long long answer = sumAbs - best;
        return answer;
    }
};

/* The required function ------------------------------------------------ */
long long get_cost(vector<int> X, vector<int> P) {
    MonumentsSolver solver;
    return solver.get_cost(std::move(X), std::move(P));
}
}

/*==========================================================================*/
/* solutions/subtask8_monuments.cpp                                        */
/*==========================================================================*/
namespace Subtask8 {
#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = (1LL<<60);

struct Entry {
    int64 d;   // distance (>0)
    int type;  // 1 = forced negative, 2 = forced positive
};

long long get_cost(vector<int> X, vector<int> P) {
    int N = (int)X.size();
    int Manc = (int)P.size();               // number of ancient monuments
    int Mmove = N - Manc;                    // number of movable monuments

    vector<char> isAnc(N, 0);
    for (int idx : P) isAnc[idx] = 1;

    /* movable coordinates, already sorted because X is sorted */
    vector<int64> O;
    O.reserve(Mmove);
    for (int i = 0; i < N; ++i)
        if (!isAnc[i]) O.push_back(X[i]);

    /* counts of ancient monuments at each positive distance */
    unordered_map<int64,int> cntPos, cntNeg;
    int64 ancZero = 0;
    for (int idx : P) {
        int64 x = X[idx];
        if (x > 0) ++cntPos[x];
        else if (x < 0) ++cntNeg[-x];
        else ++ancZero;                     // not needed for the algorithm
    }

    /* build forced slots */
    vector<Entry> base;          // forced slots
    int forcedNeg = 0, forcedPos = 0;
    // collect all distances that appear on either side
    std::set<int64> allDist;
    for (auto &kv : cntPos) allDist.insert(kv.first);
    for (auto &kv : cntNeg) allDist.insert(kv.first);

    for (int64 d : allDist) {
        int a = cntPos.count(d) ? cntPos[d] : 0;   // ancient at +d
        int b = cntNeg.count(d) ? cntNeg[d] : 0;   // ancient at -d
        int diff = a - b;
        if (diff > 0) {
            for (int i = 0; i < diff; ++i) base.push_back({d, 1}); // forced negative
            forcedNeg += diff;
        } else if (diff < 0) {
            for (int i = 0; i < -diff; ++i) base.push_back({d, 2}); // forced positive
            forcedPos += -diff;
        }
    }

    int K = forcedNeg + forcedPos;               // total forced movable monuments
    if (K > Mmove) return -1;                    // impossible

    /* sort forced slots by decreasing distance */
    sort(base.begin(), base.end(),
         [](const Entry& a, const Entry& b){ return a.d > b.d; });
    int B = (int)base.size();

    /* prefix number of forced negatives */
    vector<int> prefNeg(B + 1, 0);
    for (int i = 0; i < B; ++i)
        prefNeg[i+1] = prefNeg[i] + (base[i].type == 1);

    int maxE = (Mmove - K) / 2;                  // maximal number of extra pairs

    /* prefix sums of absolute values of O (for zero cost later) */
    vector<int64> prefAbs(Mmove + 1, 0);
    for (int i = 0; i < Mmove; ++i)
        prefAbs[i+1] = prefAbs[i] + llabs(O[i]);

    /* DP table: dp[i][e] – processed i forced slots, inserted e extra pairs */
    vector<vector<int64>> dp(B + 1, vector<int64>(maxE + 1, INF));
    dp[0][0] = 0;

    for (int i = 0; i <= B; ++i) {
        for (int e = 0; e <= maxE; ++e) {
            int64 cur = dp[i][e];
            if (cur == INF) continue;

            int usedNeg = prefNeg[i];
            int usedPos = i - usedNeg;
            int64 L = (int64)usedNeg + e;                         // index of leftmost free element
            int64 R = (int64)Mmove - 1 - ((int64)usedPos + e);   // index of rightmost free element

            /* insert an extra pair */
            if (e < maxE && L < R) {
                int64 cost = llabs(O[L] + O[R]);   // Lemma 1
                if (dp[i][e+1] > cur + cost) dp[i][e+1] = cur + cost;
            }

            /* process next forced slot */
            if (i < B) {
                const Entry &en = base[i];
                if (en.type == 1) {                // forced negative
                    if (L < Mmove) {
                        int64 cost = llabs(O[L] + en.d);   // formula (3)
                        if (dp[i+1][e] > cur + cost) dp[i+1][e] = cur + cost;
                    }
                } else {                            // forced positive
                    if (R >= 0) {
                        int64 cost = llabs(O[R] - en.d);   // formula (4)
                        if (dp[i+1][e] > cur + cost) dp[i+1][e] = cur + cost;
                    }
                }
            }
        }
    }

    int64 answer = INF;
    for (int e = 0; e <= maxE; ++e) {
        int64 cur = dp[B][e];
        if (cur == INF) continue;
        int64 leftIdx  = (int64)forcedNeg + e;
        int64 rightIdx = (int64)Mmove - 1 - ((int64)forcedPos + e);
        int64 zeroCost = 0;
        if (leftIdx <= rightIdx)
            zeroCost = prefAbs[rightIdx + 1] - prefAbs[leftIdx];
        answer = min(answer, cur + zeroCost);
    }

    return (answer == INF ? -1 : answer);
}
}

/*==========================================================================*/
/* solutions/subtask9_monuments.cpp                                        */
/*==========================================================================*/
namespace Subtask9 {
#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

/*---------------------------------------------------------------*/
/*  helper to obtain the sum of the first 'cnt' elements of a     */
/*  diagonal (stored as cumulative prefix sums)                  */
/*---------------------------------------------------------------*/
static inline int64 prefSum(const vector<int64> &pref, int cnt,
                            int64 total) {
    if (cnt <= 0) return 0;
    if (cnt <= (int)pref.size()) return pref[cnt - 1];
    return total;                     // whole diagonal is shorter
}

/*---------------------------------------------------------------*/
/*  helper to obtain the sum of the last 'cnt' elements of a      */
/*  diagonal (stored as cumulative suffix sums)                  */
/*---------------------------------------------------------------*/
static inline int64 suffSum(const vector<int64> &suf, int cnt,
                            int64 total) {
    if (cnt <= 0) return 0;
    if (cnt <= (int)suf.size()) return suf[cnt - 1];
    return total;                     // whole diagonal is shorter
}

/*---------------------------------------------------------------*/
/*  main function required by the grader                         */
/*---------------------------------------------------------------*/
long long get_cost(vector<int> X, vector<int> P) {
    const int N = (int)X.size();
    const int M = (int)P.size();          // number of ancient stones

    /*-----------------------------------------------------------*/
    /*  1. separate ancient and movable stones                  */
    /*-----------------------------------------------------------*/
    vector<char> isAnc(N, 0);
    for (int id : P) isAnc[id] = 1;

    vector<int64> posVals, negVals;       // absolute values, descending
    for (int i = 0; i < N; ++i) if (!isAnc[i]) {
        if (X[i] >= 0) posVals.push_back((int64)X[i]);
        else           negVals.push_back(-(int64)X[i]);
    }
    sort(posVals.begin(), posVals.end(), greater<int64>());
    sort(negVals.begin(), negVals.end(), greater<int64>());
    const int np = (int)posVals.size();
    const int nq = (int)negVals.size();

    /*-----------------------------------------------------------*/
    /*  2. demands caused by ancient monuments                  */
    /*-----------------------------------------------------------*/
    unordered_map<int64,int> ancPos, ancNeg;
    ancPos.reserve(M*2);
    ancNeg.reserve(M*2);
    for (int id : P) {
        int64 x = X[id];
        if (x > 0) ++ancPos[x];
        else if (x < 0) ++ancNeg[-x];
    }

    vector<int64> demandPos, demandNeg;   // positive   /   negative side demands
    for (auto &kv : ancPos) {
        int64 v = kv.first;
        int a = kv.second;
        int b = ancNeg.count(v) ? ancNeg[v] : 0;
        for (int k = 0; k < a - b; ++k) demandNeg.push_back(v);
    }
    for (auto &kv : ancNeg) {
        int64 v = kv.first;
        int b = kv.second;
        int a = ancPos.count(v) ? ancPos[v] : 0;
        for (int k = 0; k < b - a; ++k) demandPos.push_back(v);
    }

    sort(demandPos.begin(), demandPos.end(), greater<int64>());
    sort(demandNeg.begin(), demandNeg.end(), greater<int64>());

    const int sp = (int)demandPos.size();   // #positive demands
    const int sn = (int)demandNeg.size();   // #negative demands
    const int S  = sp + sn;                 // total number of demanded stones

    if ((int)(posVals.size() + negVals.size()) < S) return -1;   // not enough stones

    /*-----------------------------------------------------------*/
    /*  3. profit coming from fulfilling demands                 */
    /*-----------------------------------------------------------*/
    const int aMax = min(sp, np);
    const int bMax = min(sn, nq);

    vector<int64> prefPosDemand(aMax + 1, 0);
    for (int i = 1; i <= aMax; ++i)
        prefPosDemand[i] = prefPosDemand[i-1] + min(posVals[i-1], demandPos[i-1]);

    vector<int64> prefNegDemand(bMax + 1, 0);
    for (int i = 1; i <= bMax; ++i)
        prefNegDemand[i] = prefNegDemand[i-1] + min(negVals[i-1], demandNeg[i-1]);

    /*-----------------------------------------------------------*/
    /*  4. constants needed for the final formula                */
    /*-----------------------------------------------------------*/
    int64 CONST = 0;
    for (int64 v : posVals) CONST += v;
    for (int64 v : negVals) CONST += v;

    int64 SUMD = 0;
    for (int64 v : demandPos) SUMD += v;
    for (int64 v : demandNeg) SUMD += v;

    /*-----------------------------------------------------------*/
    /*  5. diagonal preprocessing (offsets d = a-b)             */
    /*-----------------------------------------------------------*/
    const int LIM = M;               // we never need more than M elements
    const int OFF = M;               // shift to make indices non‑negative
    const int Dcnt = 2 * M + 1;      // number of different offsets

    vector<int64> totalDiag(Dcnt, 0);        // whole sum of the diagonal
    vector<vector<int64>> prefDiag(Dcnt);   // prefix sums (first ≤ LIM elements)
    vector<vector<int64>> suffDiag(Dcnt);   // suffix sums (last  ≤ LIM elements)

    deque<int64> dq;                 // current diagonal values
    int64 curSum = 0;

    // start with d = -M
    for (int d = -M; d <= M; ++d) {
        int idx = d + OFF;

        // ----- adjust deque for the new offset d -----
        // i_start = max(0,d)   j_start = max(0,-d)
        // previous offset = d-1 (already processed)
        if (d != -M) {
            int prev_d = d - 1;
            int prev_i_start = max(0, prev_d);
            int cur_i_start  = max(0, d);
            int popCnt = cur_i_start - prev_i_start;          // 0 or 1
            for (int k = 0; k < popCnt && !dq.empty(); ++k) {
                curSum -= dq.front();
                dq.pop_front();
            }

            int prev_i_end = min(np - 1, (int)negVals.size() - 1 + prev_d);
            int cur_i_end  = min(np - 1, (int)negVals.size() - 1 + d);
            for (int i = prev_i_end + 1; i <= cur_i_end; ++i) {
                int j = i - d;                // index inside neg[]
                int64 val = min(posVals[i], negVals[j]);
                dq.push_back(val);
                curSum += val;
            }
        } else {
            // initial diagonal (d = -M)
            int i_start = max(0, d);
            int i_end   = min(np - 1, (int)negVals.size() - 1 + d);
            if (i_start <= i_end) {
                for (int i = i_start; i <= i_end; ++i) {
                    int j = i - d;
                    int64 val = min(posVals[i], negVals[j]);
                    dq.push_back(val);
                    curSum += val;
                }
            }
        }

        // ----- store whole sum and short prefix / suffix -----
        totalDiag[idx] = curSum;
        int len = (int)dq.size();

        int prefSize = min(len, LIM);
        prefDiag[idx].resize(prefSize);
        for (int i = 0; i < prefSize; ++i) {
            prefDiag[idx][i] = dq[i] + (i ? prefDiag[idx][i-1] : 0);
        }

        int suffSize = min(len, LIM);
        suffDiag[idx].resize(suffSize);
        for (int i = 0; i < suffSize; ++i) {
            suffDiag[idx][i] = dq[len-1-i] + (i ? suffDiag[idx][i-1] : 0);
        }
    }

    /*-----------------------------------------------------------*/
    /*  6. enumerate all feasible (a,b) and keep maximal profit */
    /*-----------------------------------------------------------*/
    int64 bestProfit = -1;

    for (int a = 0; a <= aMax; ++a) {
        for (int b = 0; b <= bMax; ++b) {
            if (sp - a > nq - b) continue;          // not enough negative stones
            if (sn - b > np - a) continue;          // not enough positive stones

            int64 profit = prefPosDemand[a] + prefNegDemand[b];

            int diff = a - b;                        // diagonal offset
            int idx = diff + OFF;

            int offset = (diff >= 0) ? b : a;        // min(a,b)
            int wholeLen = min(np - a, nq - b);      // length after removing prefixes

            if (wholeLen > 0) {
                int cut;
                if (np - a <= nq - b)               // positive side is limiting
                    cut = sn - b;
                else
                    cut = sp - a;

                int64 pref = prefSum(prefDiag[idx], offset, totalDiag[idx]);
                int64 suff = suffSum(suffDiag[idx], cut,    totalDiag[idx]);
                profit += totalDiag[idx] - pref - suff;
            }

            if (profit > bestProfit) bestProfit = profit;
        }
    }

    if (bestProfit < 0) return -1;                  // no feasible (a,b)

    int64 answer = CONST + SUMD - 2 * bestProfit;
    return answer;
}
}

/*==========================================================================*/
/* solutions/subtask10_monuments.cpp                                       */
/*==========================================================================*/
namespace Subtask10 {
#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

long long get_cost(vector<int> X, vector<int> P) {
    int N = (int)X.size();
    int M = (int)P.size();

    // --------------------------------------------------------------------
    // 1. mark ancient monuments
    vector<char> isAnc(N, 0);
    for (int id : P) isAnc[id] = 1;

    // --------------------------------------------------------------------
    // 2. mandatory slots (absolute values, one side only)
    unordered_map<int64, int64> diff;
    diff.reserve(N * 2);
    for (int i = 0; i < N; ++i) if (isAnc[i]) {
        int64 x = X[i];
        if (x > 0) diff[x] += 1;
        else if (x < 0) diff[-x] -= 1;               // negative side contributes -1
    }

    vector<int64> posSlot;   // mandatory slots on the positive side (+v)
    vector<int64> negSlot;   // mandatory slots on the negative side (-v)
    int64 sumSlots = 0;
    int64 base = 0;          // total number of mandatory slots

    for (auto &kv : diff) {
        int64 v = kv.first;         // absolute value
        int64 d = kv.second;        // ancPos - ancNeg
        if (d > 0) {                // need d slots on the negative side
            for (int64 i = 0; i < d; ++i) negSlot.push_back(v);
            base += d;
            sumSlots += d * v;
        } else if (d < 0) {         // need -d slots on the positive side
            int64 cnt = -d;
            for (int64 i = 0; i < cnt; ++i) posSlot.push_back(v);
            base += cnt;
            sumSlots += cnt * v;
        }
    }

    // --------------------------------------------------------------------
    // 3. collect movable sources
    vector<int64> posSrc, negSrc;
    int64 zeros = 0;
    int64 sumAbs = 0;                     // Σ |X|
    for (int i = 0; i < N; ++i) if (!isAnc[i]) {
        int64 x = X[i];
        if (x > 0) {
            posSrc.push_back(x);
            sumAbs += x;
        } else if (x < 0) {
            negSrc.push_back(-x);
            sumAbs += -x;
        } else {
            ++zeros;
        }
    }

    // --------------------------------------------------------------------
    // 4. feasibility test
    int64 totalMovable = (int64)posSrc.size() + (int64)negSrc.size() + zeros;
    if (totalMovable < base) return -1;            // impossible

    // --------------------------------------------------------------------
    // 5. sort descending (largest first)
    auto desc = greater<int64>();
    sort(posSrc.begin(),   posSrc.end(),   desc);
    sort(negSrc.begin(),   negSrc.end(),   desc);
    sort(posSlot.begin(),  posSlot.end(),  desc);
    sort(negSlot.begin(),  negSlot.end(),  desc);

    size_t iPos = 0, iNeg = 0, iPosSlot = 0, iNegSlot = 0;
    int64 totalReward = 0;

    // --------------------------------------------------------------------
    // 6. greedy loop – fill mandatory slots or create pairs
    while (iPosSlot < posSlot.size() || iNegSlot < negSlot.size()) {
        int64 remPosSrc   = (int64)posSrc.size()   - (int64)iPos;
        int64 remNegSrc   = (int64)negSrc.size()   - (int64)iNeg;
        int64 remPosSlot  = (int64)posSlot.size()  - (int64)iPosSlot;
        int64 remNegSlot  = (int64)negSlot.size()  - (int64)iNegSlot;

        // number of objects that will stay unused after every slot gets a source
        int64 extra = (remPosSrc + remNegSrc + zeros) - (remPosSlot + remNegSlot);

        // possible immediate rewards
        int64 rewardPosSlot = -1, rewardNegSlot = -1, rewardCross = -1;

        if (remPosSrc > 0 && remPosSlot > 0) {
            // largest source ≤ slot value ?
            if (posSrc[iPos] <= posSlot[iPosSlot])
                rewardPosSlot = posSrc[iPos];
            else
                rewardPosSlot = posSlot[iPosSlot];   // source > slot
        }
        if (remNegSrc > 0 && remNegSlot > 0) {
            if (negSrc[iNeg] <= negSlot[iNegSlot])
                rewardNegSlot = negSrc[iNeg];
            else
                rewardNegSlot = negSlot[iNegSlot];
        }
        if (extra >= 2 && iPos < posSrc.size() && iNeg < negSrc.size())
            rewardCross = min(posSrc[iPos], negSrc[iNeg]);

        // choose the action with the largest immediate reward
        if (rewardCross > rewardPosSlot && rewardCross > rewardNegSlot) {
            // pair a positive source with a negative source
            totalReward += rewardCross;
            ++iPos; ++iNeg;
        } else if (rewardPosSlot >= rewardNegSlot && rewardPosSlot >= 0) {
            // fill a positive mandatory slot with a positive source
            totalReward += rewardPosSlot;
            ++iPos; ++iPosSlot;
        } else if (rewardNegSlot >= 0) {
            // fill a negative mandatory slot with a negative source
            totalReward += rewardNegSlot;
            ++iNeg; ++iNegSlot;
        } else {
            // no source of the required sign is left
            // we must use a zero (if any) or an opposite‑sign source
            if (remPosSlot > 0 && zeros > 0) {
                // use a zero for a positive slot
                --zeros;
                ++iPosSlot;
            } else if (remNegSlot > 0 && zeros > 0) {
                // use a zero for a negative slot
                --zeros;
                ++iNegSlot;
            } else if (remPosSlot > 0 && iNeg < negSrc.size()) {
                // use a negative source for a positive slot (reward 0)
                ++iNeg; ++iPosSlot;
            } else if (remNegSlot > 0 && iPos < posSrc.size()) {
                // use a positive source for a negative slot (reward 0)
                ++iPos; ++iNegSlot;
            } else {
                // should not happen because of the feasibility test
                break;
            }
        }
    }

    // --------------------------------------------------------------------
    // 7. remaining slots (if any) are filled with zeros
    int64 remainingSlots = (int64)posSlot.size() - (int64)iPosSlot
                         + (int64)negSlot.size() - (int64)iNegSlot;
    if (remainingSlots > zeros) return -1;   // not enough zeros – should not occur
    // zeros are simply discarded; they give no reward

    // --------------------------------------------------------------------
    // 8. pair the still unused positive and negative sources
    size_t posRem = posSrc.size() - iPos;
    size_t negRem = negSrc.size() - iNeg;
    size_t pairs = min(posRem, negRem);
    for (size_t k = 0; k < pairs; ++k)
        totalReward += min(posSrc[iPos + k], negSrc[iNeg + k]);

    // --------------------------------------------------------------------
    // 9. final answer
    int64 answer = sumAbs + sumSlots - 2 * totalReward;
    return answer;
}
}

/*==========================================================================*/
/* Dispatcher                                                               */
/*==========================================================================*/
long long get_cost(std::vector<int> X, std::vector<int> P) {
    int N = (int)X.size();
    int M = (int)P.size();

    // Subtask 1: all monuments are ancient
    if (M == N) return Subtask1::get_cost(std::move(X), std::move(P));

    // Subtask 2: no ancient monuments
    if (M == 0) return Subtask2::get_cost(std::move(X), std::move(P));

    // Subtask 3: all ancient monuments are on the negative side
    bool allAncNeg = true;
    for (int idx : P) {
        if (X[idx] >= 0) { allAncNeg = false; break; }
    }
    if (allAncNeg) return Subtask3::get_cost(std::move(X), std::move(P));

    // Subtask 4: tiny N
    if (N <= 10) return Subtask4::get_cost(std::move(X), std::move(P));

    // Subtask 5
    if (N <= 19) return Subtask5::get_cost(std::move(X), std::move(P));

    // Subtask 6
    if (N <= 32) return Subtask6::get_cost(std::move(X), std::move(P));

    // Subtask 7 (N ≤ 200) – use the Subtask 8 solution as noted in the statement
    if (N <= 200) return Subtask8::get_cost(std::move(X), std::move(P));

    // Subtask 8
    if (N <= 4000) return Subtask8::get_cost(std::move(X), std::move(P));

    // Subtask 9 (M ≤ 4000)
    if (M <= 4000) return Subtask9::get_cost(std::move(X), std::move(P));

    // Subtask 10 – general case (no additional constraints)
    return Subtask10::get_cost(std::move(X), std::move(P));
}