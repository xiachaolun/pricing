
#ifndef _CHECK_ACCEPTANCE__
#define _CHECK_ACCEPTANCE__

#include "max_flow.h"

#include <cmath>


bool isAcceptable(const vector<vector<int> >& segment_to_user,
                  const vector<pair<int, int> >& request_list) {
    PushRelabel max_flow_solver = PushRelabel(2+N+M);
    
    int n_required_users = 0;
    for (const auto& p : request_list) {
        max_flow_solver.AddEdge(0, 2+p.first, p.second); // add edge from the source to a segment
        n_required_users += p.second;
    }
    
    for (int i = 0; i < N; i++) {
        for (const auto& idx : segment_to_user[i]) {
            max_flow_solver.AddEdge(i+2, N+2+idx, 1); // add edge from a segment to a user
        }
    }
    
    for (int i = 0; i < M; i++) {
        max_flow_solver.AddEdge(2+N+i, 1, 1); // add edge from a single user to target
    }
    
    double flow_amount = max_flow_solver.GetMaxFlow(0,1);
    
    return fabs(n_required_users - flow_amount) < 1e-6;
}

#endif
