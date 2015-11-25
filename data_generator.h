//
//  greedy_algorithm.cpp
//  
//
//  Created by Chaolun Xia on 11/24/15.
//
//

#ifndef __DATA_GENERATOR__
#define __DATA_GENERATOR__

#include <iostream>
#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include <set>
#include <algorithm>
#include <random>

using namespace std;

const int S = 0;
const int T = 1;
const int N = 36; // number of segments
const int M = 300; // number of users

int getSegmentIndex(int a, int b, int c) {
    // a,b in {0,1,2}, c in {0,1,2,3}
    return a + b * 3 + c * 9;
}

set<int> generateIndex() {
    set<int> segment_index;
    segment_index.clear();
    
    int a = rand()%3, b = rand()%3, c = rand()%4;
    segment_index.insert(getSegmentIndex(a,b,c));
    segment_index.insert(getSegmentIndex(a,b,0));
    segment_index.insert(getSegmentIndex(a,0,c));
    segment_index.insert(getSegmentIndex(0,b,c));
    segment_index.insert(getSegmentIndex(a,0,0));
    segment_index.insert(getSegmentIndex(0,b,0));
    segment_index.insert(getSegmentIndex(0,0,c));
    segment_index.insert(getSegmentIndex(0,0,0));
    
    return segment_index;
}

vector<vector<int> > generateRandomAssigment() {
    vector<vector<int> > segment_to_user;
    segment_to_user.clear();
    
    for (int i = 0; i < N; i++) {
        segment_to_user.push_back(vector<int> (0));
    }
    
    for (int i = 0; i < M; i++) {
        //max_flow_solver.AddEdge(2+N+i, 1, 1); // add an edges from j in {M} users to T
        for (const auto& idx : generateIndex()) {
            //max_flow_solver.AddEdge(2+idx, 2+N+i, 1); // add an edge from i in {N} to j in {M}
            segment_to_user[idx].push_back(i);
        }
    }
    return segment_to_user;
}

vector<pair<int, int> > generateRandomRequests(const vector<vector<int> >& segment_to_user) {
    vector<int> index_list(0);
    for (int i = 0; i < N; i++) {
        index_list.push_back(i);
    }
    random_device rd;
    mt19937 g(rd());
    shuffle(index_list.begin(), index_list.end(), g);
    index_list = vector<int> (index_list.begin(), index_list.begin() + 9);
    
    vector<pair<int, int> > request_list(0);
    for (const auto& idx : index_list) {
        int n = segment_to_user[idx].size();
        if (n == 0) {
            n = 1;
        }
        pair<int, int> p = make_pair(idx, rand()%n + 1);
        request_list.push_back(p);
    }
    sort(request_list.begin(), request_list.end());
    return request_list;
}

vector<double> generateRandomPrices() {
    vector<double> price_list(0);
    for (int i = 0; i < N; i++) {
        price_list.push_back((1+rand()%1000)/50.0);
    }
    return price_list;
}

#endif