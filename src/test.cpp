#include "data_generator.h"
#include "max_flow.h"
#include "min_cost_flow.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <assert.h>
#include <string>
#include <time.h>
#include <set>

int main() {
    srand(time(NULL));
    
    int N = 1102;
    int s = 0, t = N - 1;
    
    PushRelabel g(N);
    MinCostMaxFlow g2(N);
    for (int i = 1; i < N/2; ++i) {
        g.AddEdge(s, i, 1);
        g2.AddEdge(s, i, 1, 1);
    }
    for (int i = N/2; i < t; ++i) {
        g.AddEdge(i, t, 3);
        g2.AddEdge(i, t, 3, 1);
    }
    
    set<pair<int, int> > E;
    E.clear();
    for (int e = 0; e < N*3; ++e) {
        int v1 = rand()%(N/2-1)+1;
        int v2 = N/2 + rand()%(t - N/2);
        assert(v1 > 0);
        assert(v1 < N/2);
        assert(v2 >= N/2);
        assert(v2 < t);
        pair<int,int> p(v1,v2);
        if (E.find(p) == E.end()) {
            E.insert(p);
        } else {
            continue;
        }
        g.AddEdge(v1, v2, 1);
        g2.AddEdge(v1, v2 , 1, 1);
    }
    
    cout << g.GetMaxFlow(s,t) << endl;
    clock_t t1,t2;
    t1 = clock();
    const auto &r = g2.GetMaxFlow(s,t);
    t2 = clock();
    cout << r.first << " " << r.second << endl;
    float diff ((float)t2-(float)t1);
    cout<< diff/1000000 << endl;
    
}