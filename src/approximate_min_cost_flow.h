// Implementation of min cost max flow algorithm using adjacency
// matrix (Edmonds and Karp 1972).  This implementation keeps track of
// forward and reverse edges separately (so you can set cap[i][j] !=
// cap[j][i]).  For a regular max flow, set all edge costs to 0.
//
// Running time, O(|V|^2) cost per augmentation
//     max flow:           O(|V|^3) augmentations
//     min cost max flow:  O(|V|^4 * MAX_EDGE_COST) augmentations
//
// INPUT:
//     - graph, constructed using AddEdge()
//     - source
//     - sink
//
// OUTPUT:
//     - (maximum flow value, minimum cost value)
//     - To obtain the actual flow, look at positive values only.

#ifndef __APPROXIMATE_MIN_COST_FLOW__
#define __APPROXIMATE_MIN_COST_FLOW__

#include "data_generator.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <list>
#include <set>
#include <assert.h>
#include <time.h>
#include <cstring>

using namespace std;

typedef tuple<int, int, int> NEW_USER; //[0]: used; [1]: how many labels remaining; [2]: the largest price of other labels

bool compareRequest(const Request& r1, const Request& r2) {
    if (get<2>(r1) != get<2>(r2)) {
        return get<2>(r1) > get<2>(r2); //first compare v
    } else {
        return get<1>(r1) > get<1>(r2); //then compare d
    }
}

bool compareSelectedUser(const pair<int,int>& p1, const pair<int,int>& p2) {
    return p1.second < p2.second;
}


//the notation in this file is slightly inconsistent with those in other files.

struct ApproximateAlgorithm {
    vector<User> users;
    vector<Request> requests;
    vector<int> pricing;
    int N; // number of users;
    int M; // number of requests;
    int L; // number of labels;
    vector<vector<int> > label_user_list;
    bool used_users[MAX_USER];
    bool satisfiable_labels[MAX_BUYER][MAX_LABEL]; //satisfiable_labels[i][j] denote after i-th buyer (included), whether any label will be used.
    
    ApproximateAlgorithm(const vector<User>& users, const vector<Request>& requests, const vector<int>& pricing) {
        setParameters(users, requests, pricing);
        srand(unsigned(time(0)));
    }
    
    ApproximateAlgorithm(const NetworkData& data, const vector<int>& pricing) {
        users = data.users;
        requests = data.requests;
        this->pricing = pricing;
        N = data.M; //************ not bug!!!!! because of wrong notations
        M = data.N; //************ not bug!!!!! because of wrong notations
        L = data.L;
        srand(unsigned(time(0)));
    }
    
    ApproximateAlgorithm() {}
    
    void setParameters(const vector<User>& users, const vector<Request>& requests, const vector<int>& pricing) {
        this->users = users;
        this->requests = requests;
        this->pricing = pricing;
        N = users.size();
        M = requests.size();
        L = pricing.size();
        
        assert(L<=MAX_LABEL);
        assert(M<=MAX_BUYER);
        assert(N<=MAX_USER);
    }
    
    void _init() {
        // remap user
        label_user_list.clear();
        for (int i = 0; i < L; ++i) {
            label_user_list.push_back(vector<int> (0));
        }
        for (int i = 0; i < N; ++i) {
            for (const auto& l : users[i]) {
                label_user_list[l].push_back(i);
            }
        }
        
        sort(requests.begin(), requests.end(), compareRequest);
        
        memset(used_users, 0, sizeof(bool) * N);
        
        for (int i = 0; i < M; ++i) {
            memset(satisfiable_labels[i], 0, sizeof(bool) * L);
            for (int j = i; j < M; ++j) {
                int l = get<0>(requests[j]);
                satisfiable_labels[i][l] = 1;
            }
        }
        
    }
    
    int computeRevenue() {
//        int r1 = _computeRevenueWithLeastPrice();
        int r2 = _computeRevenueWithLeastLabels();
        int r3 = 0;
        for (int i = 0; i < 10; i++) {
//            int r = _computeRevenueWithRandomSelection();
//            r3 = max(r3, r);
        }
//        if (r1 == max(max(r1,r2), r3)) {
//            cout << 1 << endl;
//        } else if (r2 == max(max(r1,r2), r3)) {
//            cout << 2 << endl;
//        } else if (r3 == max(max(r1,r2), r3)) {
//            cout << 3 << endl;
//        }
        
//        return max(max(r1,r2), r3);
        return r2;
    }
    
    int _computeRevenueWithLeastPrice() {
        _init();
        
        int revenue = 0;
        
        vector<pair<int, int> > selected_users;

        for (int k = 0; k < M; ++k) {
            int l = get<0>(requests[k]);
            int d = get<1>(requests[k]);
            int v = get<2>(requests[k]);
            if (v < pricing[l]) continue;
            selected_users.clear();
            for (int i = 0; i < label_user_list[l].size(); ++i) {
                int u = label_user_list[l][i];
                if (used_users[u]) continue;
                int max_other_price = 0;
                int satisfiable_label_count = 0;
                for (const auto& label : users[u]) {
                    if (satisfiable_labels[k][label]) {
                        satisfiable_label_count++;
                        assert(label < L);
                        if (label != l && pricing[label] > max_other_price)
                            max_other_price = pricing[label];
                    }
                }
                if (satisfiable_label_count == 1 && d > 0) {
                    // if the user can be used for this request only, then we just use the user.
                    revenue += pricing[l];
                    d--;
                    used_users[u] = 1;
                } else {
                    selected_users.push_back(make_pair(u, max_other_price));
                }
            }
            sort(selected_users.begin(), selected_users.end(), compareSelectedUser);
            int i = 0;
            while (d > 0 && i < selected_users.size()) {
                revenue += pricing[l];
                assert(!used_users[selected_users[i].first]);
                used_users[selected_users[i].first] = 1;
                d--;
                i++;
            }
            label_user_list[l].clear();
            for (;i < selected_users.size(); ++i) {
                label_user_list[l].push_back(selected_users[i].first);
            }
        }
        return revenue;
    }
    
    int _computeRevenueWithLeastLabels() {
        _init();
        
        int revenue = 0;
        
        vector<pair<int, int> > selected_users;
        
        for (int k = 0; k < M; ++k) {
            int l = get<0>(requests[k]);
            int d = get<1>(requests[k]);
            int v = get<2>(requests[k]);
            if (v < pricing[l]) continue;
            selected_users.clear();
            for (int i = 0; i < label_user_list[l].size(); ++i) {
                int u = label_user_list[l][i];
                if (used_users[u]) continue;
                int max_other_price = 0;
                int satisfiable_label_count = 0;
                for (const auto& label : users[u]) {
                    if (satisfiable_labels[k][label]) {
                        satisfiable_label_count++;
                        assert(label < L);
                        if (label != l && pricing[label] > max_other_price)
                            max_other_price = pricing[label];
                    }
                }
                if (satisfiable_label_count == 1 && d > 0) {
                    // if the user can be used for this request only, then we just use the user.
                    revenue += pricing[l];
                    d--;
                    used_users[u] = 1;
                } else {
                    selected_users.push_back(make_pair(u, satisfiable_label_count));
                }
            }
            sort(selected_users.begin(), selected_users.end(), compareSelectedUser);
            int i = 0;
            while (d > 0 && i < selected_users.size()) {
                revenue += pricing[l];
                assert(!used_users[selected_users[i].first]);
                used_users[selected_users[i].first] = 1;
                d--;
                i++;
            }
            label_user_list[l].clear();
            for (;i < selected_users.size(); ++i) {
                label_user_list[l].push_back(selected_users[i].first);
            }
        }
        return revenue;
    }
    
    int _computeRevenueWithRandomSelection() {
        _init();
        
        int revenue = 0;
        vector<int> selected_users;
        srand(time(NULL));
        
        for (int k = 0; k < M; ++k) {
            int l = get<0>(requests[k]);
            int d = get<1>(requests[k]);
            int v = get<2>(requests[k]);
            if (v < pricing[l]) continue;
            selected_users.clear();
            for (int i = 0; i < label_user_list[l].size(); ++i) {
                int u = label_user_list[l][i];
                if (used_users[u]) continue;
                int satisfiable_label_count = 0;
                for (const auto& label : users[u]) {
                    if (satisfiable_labels[k][label]) {
                        satisfiable_label_count++;
                    }
                }
                if (satisfiable_label_count == 1 && d > 0) {
                    // if the user can be used for this request only, then we just use the user.
                    revenue += pricing[l];
                    d--;
                    used_users[u] = 1;
                } else {
                    selected_users.push_back(u);
                }
            }
            random_shuffle(selected_users.begin(), selected_users.end());
            int i = 0;
            while (d > 0 && i < selected_users.size()) {
                revenue += pricing[l];
                used_users[selected_users[i]] = 1;
                d--;
                i++;
            }
            label_user_list[l].clear();
            for (;i < selected_users.size(); ++i) {
                label_user_list[l].push_back(selected_users[i]);
            }
        }
        return revenue;
    }
};

#endif