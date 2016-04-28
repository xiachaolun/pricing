#include "data_generator.h"
#include "max_flow.h"
#include "min_cost_flow.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <assert.h>
#include <string>
#include <map>
#include <time.h>

using namespace std;

struct ProblemSolver {
    vector<User> users;
    vector<Request> requests;
    int N; // number of buyers;
    int M; // number of users;
    int L; // number of labels
    vector<pair<int, int> > edges;
    
    ProblemSolver(const NetworkData& data) {
        users = data.users;
        requests = data.requests;
        N = data.N;
        M = data.M;
        L = data.L;
        
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
    }
    
    bool _isUniform(const vector<int>& pricing) {
        for (int i = 0; i < pricing.size() - 1; ++i) {
            if (pricing[i] != pricing[i+1]) {
                return false;
            }
        }
        return true;
    }
    
    int getRevenue(const vector<int>& pricing) {
        bool is_uniform = _isUniform(pricing);
        cout << "Is uniform: " << is_uniform << endl;
        if (is_uniform) {
            return _getRevenueForUniformPricing(pricing[0]);
        } else {
            return _getRevenueForNonuniformPricing(pricing);
        }
    }
    
    int _getRevenueForUniformPricing(int price) {
        PushRelabel g(M+N+2);
        int s = N+M;
        int t = N+M+1;
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
        int max_revenue = -1, best_price = -1;
        for (set<int>::iterator it = prices.begin(); it != prices.end(); it++) {
            int revenue = _getRevenueForUniformPricing(*it);
            if (revenue > max_revenue) {
                max_revenue = revenue;
                best_price = *it;
            }
        }
        return make_pair(best_price, max_revenue);
    }
    
    int _getRevenueForNonuniformPricing(const vector<int>& pricing) {
        map<int,int> valid_buyers;
        vector<pair<int, int> > valid_edges;
        map<int, int> valid_users;
        
        valid_buyers.clear();
        // we need to come up a way to reduce the number of nodes
        for (int i = 0; i < N; ++i) {
            int l = get<0>(requests[i]);
            int d = get<1>(requests[i]);
            int v = get<2>(requests[i]);
            if (v >= pricing[l]) {
                valid_buyers.insert(make_pair(i, valid_buyers.size()));
            }
        }
        
        valid_edges.clear();
        valid_users.clear();
        for (const auto& e : edges) {
            if (valid_buyers.find(e.second) != valid_buyers.end()) {
                valid_edges.push_back(e);
                if (valid_users.find(e.first) == valid_users.end()) {
                    valid_users.insert(make_pair(e.first, valid_users.size()));
                }
            }
        }
        int N_ = valid_buyers.size();
        int M_ = valid_users.size();
        int s = N_+M_;
        int t = N_+M_+1;
        
        cout << "original users/buyers/edges are " << M << "/" << N << "/" << edges.size() << endl;
        cout << "valid users/buyers/edges are " << M_ << "/" << N_ << "/" << valid_edges.size() << endl;
        
        MinCostMaxFlow g(M_ + N_ +2);
        for (map<int,int>::iterator it = valid_users.begin(); it != valid_users.end(); it++) {
            g.AddEdge(s, it->second, 1, 0);
        }
        for (const auto& e : valid_edges) {
            int user_original_index = e.first;
            int buyer_original_index = e.second;
            assert(valid_users.find(user_original_index) != valid_users.end());
            assert(valid_buyers.find(buyer_original_index) != valid_buyers.end());
            g.AddEdge(valid_users[user_original_index], M_ + valid_buyers[buyer_original_index], 1, 0);
        }
        
        for (int i = 0; i < N; ++i) {
            int l = get<0>(requests[i]);
            int d = get<1>(requests[i]);
            int v = get<2>(requests[i]);
            if (v >= pricing[l]) {
                g.AddEdge(M_ + valid_buyers[i], t, d, 100 - pricing[l]); // here we assume the max price is 100.
            }
        }
        pair<int, int> r = g.GetMaxFlow(s,t);
        int flow = r.first;
        int cost = r.second;
        return 100*flow - cost;
    }
};

int main() {
    srand(time(NULL));
    NetworkData data;
    data.init();
//    data.loadFromFile("data/data_2.txt");
    ProblemSolver ps(data);
    //int p[]={4,3,3};
    vector<int> pricing(0);
    for (int i = 0; i < 10; i++) {
        pricing.push_back(rand()%100+1);
    }
    cout << ps.getRevenue(pricing) << endl;
//    pair<int, int> r = ps.findOptimalUniformPrice();
//    cout << r.first << " " << r.second << endl;
}