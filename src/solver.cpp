#include "data_generator.h"
#include "max_flow.h"
#include "min_cost_flow.h"
#include "approximate_min_cost_flow.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <assert.h>
#include <string>
#include <map>
#include <time.h>
#include <cstring>

using namespace std;

struct ProblemSolver {
    vector<User> users;
    vector<Request> requests;
    int N; // number of buyers;
    int M; // number of users;
    int L; // number of labels
    int l_size[MAX_LABEL]; // number of users with that label
//    vector<pair<int, unsigned short> > edges;
    float af[MAX_LABEL][MAX_LABEL]; // arbitrage-free matrix
//    ApproximateAlgorithm aa;
    
    //variable for optimal pricing
    int dfs_revenue;
    vector<int> dfs_pricing;
    
    ProblemSolver(const NetworkData& data) {
        users = data.users;
        requests = data.requests;
        N = data.N;
        M = data.M;
        L = data.L;
        memset(l_size, 0, sizeof(l_size));
        for (const auto& user : users) {
            for (const auto& l : user) {
                l_size[l] += 1;
            }
        }
        
        assert(L<=1000);
        
        _computeArbitrageFreeConstraints();
    }
    
    vector<tuple<int, int, int> > _genearteEdge() {
        // this process already convert the index as:
        // user k -> k; user set j -> M+j; advertiser i-> M+L+i
        vector<tuple<int, int, int> > edges;
        edges.clear();
        for (int k = 0; k < M; ++k) {
            for (const auto& l : users[k]) {
                edges.push_back(tuple<int, int, int>(k, M+l, 1));
            }
        }
        
        for (int j = 0; j < N; ++j) {
            int l = get<0>(requests[j]);
            edges.push_back(tuple<int, int, int>(M+l, M+L+j, l_size[l]));
        }
        return edges;
    }
    
    void _computeArbitrageFreeConstraints() {
        /*
         p_i|u_i|>=p_j|u_i\cap u_j| --> p_i>=p_j*af[i][j] where af[i][j]=|u_i\cap u_j|/|u_i|,similarly
         p_j[u_j|>=p_i|u_i\cap u_j| --> p_j>=p_i*af[j][i] where af[j][i]=|u_i\cap u_j|/|u_j|
         So p_i is valid iff for any k, p_i>=p_k*af[i][k] and for any k, p_k>=p_i*af[k][i]
         */
        
        memset(af, 0, sizeof(af));
        vector<vector<int> > label_user_list(0);
        for (int i = 0; i < L; ++i) {
            label_user_list.push_back(vector<int> (0));
        }
        for (int i = 0; i < users.size(); ++i) {
            for (const auto& l : users[i]) {
                label_user_list[l].push_back(i);
            }
        }
        
        for (int i = 0; i < L; ++i) {
            for (int j = i + 1; j < L; ++j) {
                vector<int> intersection(0);
                set_intersection(label_user_list[i].begin(), label_user_list[i].end(),
                                 label_user_list[j].begin(), label_user_list[j].end(),
                                 back_inserter(intersection));
                af[i][j] = intersection.size() * 1.0 / label_user_list[i].size();
                af[j][i] = intersection.size() * 1.0 / label_user_list[j].size();
            }
        }
    }
    
    bool _isUniform(const vector<int>& pricing) {
        for (int i = 0; i < pricing.size() - 1; ++i) {
            if (pricing[i] != pricing[i+1]) {
                return false;
            }
        }
        return true;
    }
    
    void debug() {
        vector<int> pricing(0);
        for (int i = 0; i < L; ++i) {
            pricing.push_back(rand()%MAX_VALUATION+1);
        }
        bool is_uniform = _isUniform(pricing);
        if (is_uniform) {
            cout << _getRevenueForUniformPricing(pricing[0]) << endl;
        } else {
            cout << _getRevenueForNonuniformPricing(pricing) << endl;
            cout << _getApproximateRevenueForNonuniformPricing(pricing) << endl;
        }
    }
    
    pair<int, int> findOptimalUniformPrice() {
        set<int> prices;
        prices.clear();
        for (int i = 0; i < N; ++i) {
            int v = get<2>(requests[i]);
            prices.insert(v);
        }
        int best_revenue = -1, best_price = -1;
        for (set<int>::iterator it = prices.begin(); it != prices.end(); it++) {
            int revenue = _getRevenueForUniformPricing(*it);
            if (revenue > best_revenue) {
                best_revenue = revenue;
                best_price = *it;
            }
        }
        return make_pair(best_revenue, best_price);
    }

    pair<int, vector<int> > findLocallyOptimalNonuiformPricing(bool use_random=0, bool include_detail=1) {
        vector<set<int> > valuations(0);
        for (int i = 0; i < L; ++i) {
            valuations.push_back(set<int> ());
        }
        for (const auto& request : requests) {
            int l = get<0>(request);
            int v = get<2>(request);
            valuations[l].insert(v);
        }
        
        pair<int, int> r = findOptimalUniformPrice();
        //cout << "revenue : " << r.first << " with uniform price: " << r.second << endl;
        int revenue = r.first; // current_revenue;
        vector<int> pricing(0); // current_pricing
        for (int l = 0; l < L; ++l) {
            pricing.push_back(r.second);
        }
        bool changed = true;
        
        int round = 0;
        if (include_detail) cout << round << ":" << revenue;
        
        while (changed ) {
            round++;
            changed = false;
            vector<pair<double, double> > bounds(0);
            for (int l = 0; l < L; ++l) {
                bounds.push_back(_computePriceLowerAndUpperBound(l, pricing));
            }
            for (int l = 0; l < L; ++l) {
                vector<int> prcing_l = pricing;
                for (set<int>::iterator it = valuations[l].begin(); it != valuations[l].end(); it++) {
                    int v = *it;
                    if (v == pricing[l] || v < bounds[l].first || v > bounds[l].second) continue;
                    prcing_l = pricing;
                    prcing_l[l] = v;
                    int new_revenue = use_random ? _getApproximateRevenueForNonuniformPricing(prcing_l) : _getRevenueForNonuniformPricing(prcing_l);
                    if (new_revenue > revenue) {
                        revenue = new_revenue;
                        changed = true;
                        pricing[l] = v;
                        if (include_detail) cout << " " << round << ":" << new_revenue;
                    }
                }
            }
        }
        if (include_detail) cout << endl;
        return make_pair(revenue, pricing);
    }
    
    pair<int, vector<int> > findOptimalPricingByDFS() {
        int P[MAX_LABEL];
        dfs_revenue = 0;
        dfs_pricing.clear();
        _DFSPricing(0, P);
        return make_pair(dfs_revenue, dfs_pricing);
    }
    
    int _getRevenueForNonuniformPricing(const vector<int>& pricing) {
        MinCostMaxFlow g(M + N + L + 2);
        int s, t; //source & sink
        s = N + M + L;
        t = N + M + L + 1;
        for (int i = 0; i < M; ++i) {
            g.AddEdge(s, i, 1, 0);
        }
        
        for (const auto& e : _genearteEdge()) {
            int from = get<0>(e);
            int to = get<1>(e);
            int cap = get<2>(e);
            g.AddEdge(from, to, cap, 0);
        }
        
        for (int i = 0; i < N; ++i) {
            int l = get<0>(requests[i]);
            int d = get<1>(requests[i]);
            int v = get<2>(requests[i]);
            if (v >= pricing[l]) {
                g.AddEdge(i+M+L, t, d, MAX_VALUATION - pricing[l]);
            }
        }
        pair<int, int> r = g.GetMaxFlow(s,t);
        int flow = r.first;
        int cost = r.second;
        return MAX_VALUATION*flow - cost;
    }
    
    int _getRevenueForUniformPricing(int price) {
        PushRelabel g(M+N+L+2);
        int s, t; //source & sink
        s = N + M + L;
        t = N + M + L + 1;
        for (int i = 0; i < M; ++i) {
            g.AddEdge(s, i, 1);
        }
        
        for (const auto& e : _genearteEdge()) {
            int from = get<0>(e);
            int to = get<1>(e);
            int cap = get<2>(e);
            g.AddEdge(from, to, cap);
        }
        for (int i = 0; i < N; ++i) {
            int d = get<1>(requests[i]);
            int v = get<2>(requests[i]);
            if (v >= price) {
                g.AddEdge(i+M+L, t, d);
            }
        }
        return g.GetMaxFlow(s,t) * price;
    }
    
    int _getApproximateRevenueForNonuniformPricing(const vector<int>& pricing) {
        ApproximateAlgorithm aa(users, requests, pricing);
        return aa.computeRevenue();
    }
    
    pair<double, double> _computePriceLowerAndUpperBound(int l, const vector<int>& pricing) {
        double lower = 1;
        double upper = MAX_VALUATION;
        for (int i = 0; i < L; ++i) {
            if (i == l) continue;
            lower = max((double)af[l][i]*pricing[i], lower);
            if (af[i][l] > 1e-6) {
                upper = min((double)pricing[i]/af[i][l], upper);
            }
        }
        return make_pair(lower, upper);
    }
    
    void _DFSPricing(int l, int P[MAX_LABEL]) {
        if (l == L) {
            vector<int> pricing(P, P + L);
            const auto& r = _getRevenueForNonuniformPricing(pricing);
            if (r > dfs_revenue) {
                dfs_revenue = r;
                dfs_pricing = pricing;
            }
        } else {
            for (int i = 1; i <= MAX_VALUATION; ++i) {
                P[l] = i;
                if (_isArbitrageFree(l+1, P)) {
                    _DFSPricing(l+1, P);
                }
            }
        }
        
    }
    
    bool _isArbitrageFree(int l, int P[MAX_LABEL]) {
        for (int i = 0; i < l; ++i) {
            for (int j = 0; j < l; ++j) {
                if (i == j) continue;
                if (P[i] < P[j] * af[i][j] || P[j] < P[i] * af[j][i])
                    return 0;
            }
        }
        return 1;
    }
};

void runEvaluation(int N, int M, int L, int L_user) {
    int n_cases = 1;
    while (true) {
        cout << "case: " << n_cases++ << endl;
        srand(unsigned(time(0)));
        NetworkData data;
        data.init(N,M,L,L_user);
        ProblemSolver ps(data);
        int nonuni_r = ps.findLocallyOptimalNonuiformPricing(1).first;
    }
    
    //    while (true) {
    //        NetworkData data;
    //        data.init(N,M,L,L_user);
    //        ProblemSolver ps(data);
    //        int uni_r = ps.findOptimalUniformPrice().first;
    //        int nonuni_r = ps.findLocallyOptimalNonuiformPricing(1, 0).first;
    //        int opt_r = ps.findOptimalPricingByDFS().first;
    //        cout << uni_r << " " << nonuni_r << " " << opt_r << endl;
    //    }
}

int main(int argc, char* argv[]) {
    assert(argc == 5);
    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    int L = atoi(argv[3]);
    int L_user = atoi(argv[4]);
    cout << "Buyers: " << N << " Users: " << M << " L: " << L << " L per user: " << L_user << " Max Valution:" << MAX_VALUATION << endl;

    runEvaluation(N,M,L,L_user);
}
