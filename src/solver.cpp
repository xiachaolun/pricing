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
    int s, t; //source & sink
    vector<pair<int, int> > edges;
    double af[MAX_LABEL][MAX_LABEL]; // arbitrage-free matrix
//    ApproximateAlgorithm aa;
    
    ProblemSolver(const NetworkData& data) {
        users = data.users;
        requests = data.requests;
        N = data.N;
        M = data.M;
        L = data.L;
        s = N + M;
        t = N + M + 1;
        
        assert(L<=1000);
        
        edges.clear();
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                int l = get<0>(requests[j]);
                assert(0 <= l);
                assert(l < L);
                if (find(users[i].begin(), users[i].end(), l) != users[i].end()) {
                    edges.push_back(make_pair(i, j));
                }
            }
        }
        _computeArbitrageFreeConstraints();
    }
    
    void _computeArbitrageFreeConstraints() {
        /*
         p_i|u_i|>=p_j|u_i\cap u_j| --> p_i>=p_j*af[i][j] where af[i][j]=|u_i\cap u_j|/|u_i|,similarly
         p_j[u_j|>=p_i|u_i\cap u_j| --> p_j>=p_i*af[j][i] where af[j][i]=|u_i\cap u_j|/|u_j|
         So p_i is valid iff for any k, p_i>=p_k*af[i][k] and for any k, p_k>=p_i*af[k][i]
         */
        
        memset(af, sizeof(af), 0);
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
    
    int _getRevenueForUniformPricing(int price) {
        PushRelabel g(M+N+2);
        for (int i = 0; i < M; ++i) {
            g.AddEdge(s, i, 1);
        }
        for (const auto& e : edges) {
            g.AddEdge(e.first, e.second + M, 1);
        }
        for (int i = 0; i < N; ++i) {
            int d = get<1>(requests[i]);
            int v = get<2>(requests[i]);
            if (v >= price) {
                g.AddEdge(i+M, t, d);
            }
        }
        return g.GetMaxFlow(s,t) * price;
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
    
    pair<int, vector<int> > findLocallyOptimalNonuiformPricing(bool use_random=0) {
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
        cout << "revenue : " << r.first << " with uniform price: " << r.second << endl;
        int best_revenue = r.first;
        vector<int> pricing(0);
        for (int l = 0; l < L; ++l) {
            pricing.push_back(r.second);
        }
        vector<int> best_pricing = pricing;
        bool changed = true;
        while (changed ) {
            changed = false;
            vector<pair<double, double> > bounds(0);
            for (int l = 0; l < L; ++l) {
                bounds.push_back(_computePriceLowerAndUpperBound(l, pricing));
            }
            int best_l = -1, best_v = -1, best_new_revenue = best_revenue;
            for (int l = 0; l < L; ++l) {
                for (set<int>::iterator it = valuations[l].begin(); it != valuations[l].end(); it++) {
                    int v = *it;
                    if (v == pricing[l] || v < bounds[l].first || v > bounds[l].second) continue;
                    vector<int> new_pricing = best_pricing; // this could be optimized later
                    new_pricing[l] = v;
                    int new_revenue = use_random ? _getApproximateRevenueForNonuniformPricing(new_pricing) : _getRevenueForNonuniformPricing(new_pricing);
                    if (new_revenue > best_new_revenue) {
                        best_l = l;
                        best_v = v;
                        best_new_revenue = new_revenue;
                    }
                }
            }
            if (best_l != -1) {
                changed = true;
                best_pricing[best_l] = best_v;
                best_revenue = best_new_revenue;
                cout << "revenue : " << best_revenue << " with prices:";
                for (int l = 0; l < L; l++) {
                    cout << " " << best_pricing[l];
                }
                cout << endl;
            }
        }
        return make_pair(best_revenue, best_pricing);
    }
    
    pair<int, vector<int> > findLocallyOptimalNonuiformPricing2(bool use_random=0) {
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
        cout << "revenue : " << r.first << " with uniform price: " << r.second << endl;
        int revenue = r.first; // current_revenue;
        vector<int> pricing(0); // current_pricing
        for (int l = 0; l < L; ++l) {
            pricing.push_back(r.second);
        }
        bool changed = true;
        while (changed ) {
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
                    }
                }
            }
            if (changed) {
                cout << "revenue : " << revenue << " with prices:";
                for (int l = 0; l < L; l++) {
                    cout << " " << pricing[l];
                }
                cout << endl;
            }
        }
        return make_pair(revenue, pricing);
    }
    
    int _getRevenueForNonuniformPricing(const vector<int>& pricing) {
        MinCostMaxFlow g(M + N + 2);
        for (int i = 0; i < M; ++i) {
            g.AddEdge(s, i, 1, 0);
        }
        for (const auto& e : edges) {
            g.AddEdge(e.first, e.second + M, 1, 0);
        }
        for (int i = 0; i < N; ++i) {
            int l = get<0>(requests[i]);
            int d = get<1>(requests[i]);
            int v = get<2>(requests[i]);
            if (v >= pricing[l]) {
                g.AddEdge(i+M, t, d, MAX_VALUATION - pricing[l]);
            }
        }
        pair<int, int> r = g.GetMaxFlow(s,t);
        int flow = r.first;
        int cost = r.second;
        return MAX_VALUATION*flow - cost;
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
            lower = max(af[l][i]*pricing[i], lower);
            if (af[i][l] > 1e-8) {
                upper = min(pricing[i]/af[i][l], upper);
            }
        }
        return make_pair(lower, upper);
    }
};

int main() {
    srand(time(NULL));
    NetworkData data;
    data.init();
//    data.loadFromFile("data/data_2.txt");
    ProblemSolver ps(data);
    int uni_r = ps.findOptimalUniformPrice().first;
    int nonuni_r = ps.findLocallyOptimalNonuiformPricing2(1).first;
    cout << (nonuni_r - uni_r)*1.0 / uni_r << endl;
//    cout << ps.findOptimalUniformPrice().first << endl;
//    cout << ps.findLocallyOptimalNonuiformPricing2(1).first << endl;
//    ps.debug();
}