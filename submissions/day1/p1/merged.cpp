/*
 * Merged solution for the Ball Machine problem.
 *
 * Four original solutions are kept verbatim (except for the global
 * declarations of `insert` and `collect`, which must appear only once
 * in the global scope).  Each solution is placed in its own namespace.
 *
 * The dispatcher `find_structure` chooses which implementation to run
 * based only on the value of M.  Subtask‑1 cannot be distinguished from
 * the other subtasks using only M, and the general solution (Subtask‑4)
 * works for it as well, so it is not invoked directly.
 */

/*======================================================================
 * Global declarations of the grader's interface.
 *====================================================================*/
#include <bits/stdc++.h>
using namespace std;

extern bool insert(int U, int X);               // provided by the grader
extern std::vector<int> collect();              // provided by the grader

/*======================================================================
 * solutions/subtask1_ballmachine.cpp
 *====================================================================*/
namespace subtask1 {
#include <bits/stdc++.h>
using namespace std;

//--- the grader provides these functions ---------------------------------
//extern bool insert(int U, int X);          // try to insert a ball
//extern vector<int> collect();              // collect and empty the machine
//-------------------------------------------------------------------------

vector<int> find_structure(int M) {
    const int BALL_VALUE = 0;               // any non‑negative value, 0 is enough
    vector<int> depth(M, 0);

    // 1. determine the depth of every leaf
    for (int leaf = 0; leaf < M; ++leaf) {
        int successes = 0;
        while (insert(leaf, BALL_VALUE))   // true → ball was inserted
            ++successes;
        // leaf is now occupied; next insert would return false
        depth[leaf] = successes - 1;        // true depth
        collect();                          // empty the machine for the next leaf
    }

    // 2. compute total number of nodes
    long long sumDepth = 0;
    for (int d : depth) sumDepth += d;
    int N = static_cast<int>(sumDepth) + 1; // total nodes, root = N - 1

    // 3. build a suitable parent array
    vector<int> parent(N - 1, -1);           // R[0 … N-2]
    int nextId = M;                         // first free index for internal nodes

    for (int leaf = 0; leaf < M; ++leaf) {
        int d = depth[leaf];
        if (d == 1) {
            // leaf is a direct child of the root
            parent[leaf] = N - 1;
        } else {
            // create a private chain of (d-1) internal nodes
            int first = nextId;              // node directly above the leaf
            parent[leaf] = first;            // leaf → first internal node
            for (int i = 0; i < d - 1; ++i) {
                int cur = first + i;
                if (i == d - 2) {            // last node of the chain
                    parent[cur] = N - 1;      // connects to the root
                } else {
                    parent[cur] = cur + 1;    // points to the next node upwards
                }
            }
            nextId += d - 1;                 // reserve those indices
        }
    }
    return parent;
}
} // namespace subtask1

/*======================================================================
 * solutions/subtask2_ballmachine.cpp
 *====================================================================*/
namespace subtask2 {
#include <bits/stdc++.h>
using namespace std;

//extern bool insert(int leaf, int value);
//extern vector<int> collect();

vector<int> find_structure(int M) {
    // ------------------------------------------------------------
    // 1. leaf depths
    vector<int> depth(M, 0);
    for (int i = 0; i < M; ++i) {
        int cnt = 0;
        while (insert(i, 0)) ++cnt;        // count successful insertions
        depth[i] = cnt - 1;                 // depth = successes - 1
        collect();                          // empty the machine
    }

    // ------------------------------------------------------------
    // 2. pairwise LCA depths
    vector<vector<int>> L(M, vector<int>(M, -1));
    for (int i = 0; i < M; ++i) {
        for (int j = i + 1; j < M; ++j) {
            // fill leaf i completely
            while (insert(i, 0)) {}
            // now insert leaf j and count successes
            int cnt = 0;
            while (insert(j, 0)) ++cnt;
            L[i][j] = depth[j] - cnt;       // Lemma 2
            L[j][i] = L[i][j];
            collect();                      // empty for the next round
        }
    }

    // ------------------------------------------------------------
    // 3. rebuild the tree (old identifiers)
    int nextId = 0;
    int rootId = nextId++;                  // id 0 is the root
    vector<vector<int>> node(M);            // node[i][d] = id of leaf i at depth d
    for (int i = 0; i < M; ++i) {
        node[i].assign(depth[i] + 1, -1);
        node[i][0] = rootId;                // root is shared
    }

    for (int i = 0; i < M; ++i) {
        for (int d = 1; d <= depth[i]; ++d) {
            int shared = -1;
            for (int j = 0; j < i; ++j) {
                if (L[i][j] >= d) {        // this depth is already present
                    shared = node[j][d];
                    break;
                }
            }
            if (shared != -1) node[i][d] = shared;
            else               node[i][d] = nextId++;
        }
    }

    int N = nextId;                         // total number of vertices

    // parent in the old numbering
    vector<int> parentOld(N, -1);
    for (int i = 0; i < M; ++i) {
        for (int d = 1; d <= depth[i]; ++d) {
            int child = node[i][d];
            int par   = node[i][d - 1];
            parentOld[child] = par;
        }
    }

    // adjacency list (children)
    vector<vector<int>> children(N);
    for (int v = 0; v < N; ++v) {
        int p = parentOld[v];
        if (p != -1) children[p].push_back(v);
    }

    // leaf nodes in old numbering
    vector<int> leafOld(M);
    for (int i = 0; i < M; ++i) leafOld[i] = node[i][depth[i]];

    // ------------------------------------------------------------
    // 4. new labeling (0..M-1 for leaves, N-1 for root)
    vector<int> newLabel(N, -1);
    for (int i = 0; i < M; ++i) newLabel[leafOld[i]] = i;   // fixed leaf labels

    int cur = M;                                   // next free label
    function<void(int)> dfs = [&](int u) {
        for (int v : children[u]) dfs(v);
        if (newLabel[u] == -1) newLabel[u] = cur++;
    };
    dfs(rootId);                                   // root gets the last label N-1

    // ------------------------------------------------------------
    // 5. build the answer array R
    vector<int> R(N - 1, -1);                      // R[child] = parent
    for (int v = 0; v < N; ++v) {
        int p = parentOld[v];
        if (p == -1) continue;                     // root
        int childLabel  = newLabel[v];
        int parentLabel = newLabel[p];
        R[childLabel] = parentLabel;
    }
    return R;
}
} // namespace subtask2

/*======================================================================
 * solutions/subtask3_ballmachine.cpp
 *====================================================================*/
namespace subtask3 {
#include <bits/stdc++.h>
using namespace std;

// forward declarations – the grader supplies the definitions
//extern bool insert(int U, int X);
//extern std::vector<int> collect();

std::vector<int> find_structure(int M) {
    const int MAX_VAL = 999;                     // B = 999, K = 1  →  C = 1000
    int cur = MAX_VAL;

    // leafValue[i] – final value of the ball that stays on leaf i
    vector<int> leafValue(M, -1);

    // -----------------------------------------------------------------
    // 1) fill the whole tree, always using a smaller value for every success
    while (true) {
        bool anySuccess = false;
        for (int leaf = 0; leaf < M; ++leaf) {
            if (cur < 0) break;            // no more admissible values
            if (insert(leaf, cur)) {       // a new vertex became occupied
                leafValue[leaf] = cur;   // remember the ball's value
                --cur;                    // next successful ball gets a smaller value
                anySuccess = true;
            }
        }
        if (!anySuccess) break;          // no vertex was newly occupied → done
    }

    // -----------------------------------------------------------------
    // 2) obtain the preorder list of all values
    vector<int> preorder = collect();     // K = 1
    int N = (int)preorder.size();         // total number of nodes ( = successes )

    // map leaf values → leaf index
    unordered_map<int,int> leafValToIdx;
    for (int i = 0; i < M; ++i) leafValToIdx[leafValue[i]] = i;

    // -----------------------------------------------------------------
    // 3) reconstruct the tree from the preorder list
    // node IDs:   leaves 0..M-1, internal nodes M, M+1, ...
    vector<int> nodeValue;                // value of each node (by ID)
    nodeValue.reserve(N);
    nodeValue.resize(M);                  // placeholders for leaves

    // fill leaf values
    for (int i = 0; i < M; ++i) nodeValue[i] = leafValue[i];

    vector<int> parentOld;                // parent of each node (old IDs)
    parentOld.reserve(N);
    parentOld.resize(M, -1);              // leaves will get their parent later

    unordered_map<int,int> internalValToId;   // value → internal node ID

    vector<int> stack;                    // stack of node IDs (current ancestors)

    int rootOld = -1;                     // old ID of the root

    for (int v : preorder) {
        int id;
        auto itLeaf = leafValToIdx.find(v);
        if (itLeaf != leafValToIdx.end()) {
            // a leaf
            id = itLeaf->second;          // leaf index is its ID
        } else {
            auto itInt = internalValToId.find(v);
            if (itInt == internalValToId.end()) {
                id = (int)nodeValue.size();   // next free ID
                internalValToId[v] = id;
                nodeValue.push_back(v);
                parentOld.push_back(-1);
            } else {
                id = itInt->second;
            }
        }

        // pop finished subtrees
        while (!stack.empty() && nodeValue[stack.back()] < v) {
            stack.pop_back();
        }
        if (!stack.empty()) {
            parentOld[id] = stack.back();
        } else {
            rootOld = id;                 // this node has no parent → the root
        }
        stack.push_back(id);
    }

    // ---------------------------------------------------------------
    // 4) relabel internal nodes: leaves keep their numbers,
    //    the root gets label N-1, the remaining internals get M … N-2
    int totalNodes = (int)nodeValue.size();   // should be N
    vector<int> oldToNew(totalNodes, -1);
    for (int i = 0; i < M; ++i) oldToNew[i] = i;   // leaves unchanged

    int nextLabel = M;
    for (int old = M; old < totalNodes; ++old) {
        if (old == rootOld) continue;
        oldToNew[old] = nextLabel++;
    }
    oldToNew[rootOld] = totalNodes - 1;    // root = N-1

    // ---------------------------------------------------------------
    // 5) build the required array R[0 … N-2]
    vector<int> R(totalNodes - 1, -1);
    for (int old = 0; old < totalNodes; ++old) {
        int newId = oldToNew[old];
        if (newId == totalNodes - 1) continue; // root – no entry in R
        int oldPar = parentOld[old];
        int newPar = (oldPar == -1 ? -1 : oldToNew[oldPar]);
        R[newId] = newPar;
    }

    return R;
}
} // namespace subtask3

/*======================================================================
 * solutions/subtask4_ballmachine.cpp
 *====================================================================*/
namespace subtask4 {
#include <bits/stdc++.h>
using namespace std;

/*  the grader supplies these two functions  */
//extern bool insert(int U, int X);
//extern vector<int> collect();

vector<int> find_structure(int M)
{
    const int MAX_VAL = 999;               // allowed ball values are 0..1000
    int cur = MAX_VAL;                     // current value for the next ball

    // leafValue[u] – the value of the ball that finally stays at leaf u
    vector<int> leafValue(M, -1);

    /* ---------------------------------------------------------
       1) Fill the whole machine.
          We try all leaves repeatedly, inserting decreasing values.
          When insert succeeds we remember the value for that leaf.
       --------------------------------------------------------- */
    while (true) {
        bool any = false;
        for (int leaf = 0; leaf < M; ++leaf) {
            if (cur < 0) break;            // no more admissible values
            if (insert(leaf, cur)) {       // a new vertex became occupied
                leafValue[leaf] = cur;     // remember the ball's value
                any = true;
                --cur;                     // next value will be smaller
            }
        }
        if (!any) break;                   // no vertex was newly occupied → done
    }

    /* ---------------------------------------------------------
       2) Obtain the traversal sequence S.
       --------------------------------------------------------- */
    vector<int> seq = collect();            // S – values of all N balls
    int N = (int)seq.size();                // number of vertices

    /* ---------------------------------------------------------
       3) Reconstruct the tree from the preorder sequence.
          The values satisfy the heap property:
          parent value > all values in its subtree.
          Hence a monotone decreasing stack works.
       --------------------------------------------------------- */
    vector<int> parentIdx(N, -1);            // parent index in the reconstructed tree
    vector<vector<int>> children(N);
    vector<int> st;                         // stack of vertex indices (indices in seq)
    int rootIdx = -1;

    for (int i = 0; i < N; ++i) {
        int v = seq[i];
        while (!st.empty() && seq[st.back()] < v)   // finish subtrees of smaller values
            st.pop_back();

        if (!st.empty()) {
            int p = st.back();
            parentIdx[i] = p;
            children[p].push_back(i);
        } else {
            parentIdx[i] = -1;               // this is the root
            rootIdx = i;
        }
        st.push_back(i);
    }

    /* ---------------------------------------------------------
       4) Build a map: value of a leaf -> leaf index (0..M-1).
       --------------------------------------------------------- */
    unordered_map<int,int> leafIdByValue;
    leafIdByValue.reserve(M * 2);
    for (int u = 0; u < M; ++u) {
        // each leaf is finally occupied, leafValue[u] must be set
        leafIdByValue[ leafValue[u] ] = u;
    }

    /* ---------------------------------------------------------
       5) Assign new labels.
           - leaves keep their original indices 0..M-1
           - internal vertices (except root) get M .. N-2
           - root gets N-1
       --------------------------------------------------------- */
    vector<int> newId(N, -1);

    // leaves: use the original leaf numbers obtained from leafValue mapping
    for (int i = 0; i < N; ++i) {
        if (children[i].empty()) {                  // a leaf in the real tree
            int val = seq[i];
            auto it = leafIdByValue.find(val);
            // it must exist because leaf values are distinct
            newId[i] = it->second;                  // leaf number 0..M-1
        }
    }

    // internal vertices (except root)
    int nextId = M;                                 // start after leaves
    for (int i = 0; i < N; ++i) {
        if (i == rootIdx) continue;
        if (!children[i].empty()) {                 // internal node
            newId[i] = nextId++;
        }
    }
    newId[rootIdx] = N - 1;                         // root label

    /* ---------------------------------------------------------
       6) Build the answer array R[0 .. N-2].
       --------------------------------------------------------- */
    vector<int> R(N - 1, -1);
    for (int i = 0; i < N; ++i) {
        if (i == rootIdx) continue;                 // root has no entry
        int child = newId[i];
        int par   = newId[parentIdx[i]];
        R[child] = par;
    }
    return R;
}
} // namespace subtask4

/*======================================================================
 * Dispatcher
 *====================================================================*/
vector<int> find_structure(int M) {
    // Subtask 2 (M ≤ 3) – the specialised tiny‑leaf solution.
    if (M <= 3) {
        return subtask2::find_structure(M);
    }
    // Subtask 3 (4 ≤ M ≤ 45) – the solution that works for moderate sizes.
    if (M <= 45) {
        return subtask3::find_structure(M);
    }
    // Subtask 4 (M > 45) – the general solution.
    // Subtask 1 cannot be distinguished using only M; the general
    // algorithm also works for the star‑shaped trees of Subtask 1,
    // so we use the Subtask 4 implementation for those cases.
    return subtask4::find_structure(M);
}