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
#include <assert.h>
#include <set>
#include <algorithm>
#include <random>
#include <utility>
#include <fstream>

using namespace std;


typedef tuple<int, int, int> Request;
typedef vector<int> User;

const int MAX_USER = 500000;
const int MAX_BUYER = 1000;
const int MAX_LABEL = 500;
const int MAX_VALUATION = 10;

struct NetworkData {
    int N; // number of buyers
    int M; // number of users
    int L; // number of labels
    int D; // number of max demand
    vector<Request> requests;
    vector<User> users;
    
    NetworkData () {
    }
    
    void init() {
        srand(time(NULL));
        _setLargeData();
        assert(N>=L);
        _generateRequests();
        _generateUsers();
    }
    
    void _setLargeData() {
        N = 1000;
        M = 100000;
        L = MAX_LABEL;
        D = M * 2 / N;
    }
    void _setMediumData() {
        N = 100;
        M = 1000;
        L = 30;
        D = M*2/N;
    }
    
    void _setSmallData() {
        N = 10;
        M = 50;
        L = 5;
        D = 10;
    }
    
    void loadFromFile(string file_name) {
        ifstream fin(file_name);
        fin >> M >> N >> L;
        
        requests.clear();
        for (int i = 0; i < N; ++i) {
            int l, d, v;
            fin >> l >> d >> v;
            requests.push_back(Request(l, d, v));
        }
        
        users.clear();
        for (int i = 0; i < M;++i) {
            int n;
            User u;
            u.clear();
            fin >> n;
            for (int j = 0; j < n; ++j) {
                int l;
                fin >> l;
                u.push_back(l);
            }
            users.push_back(u);
        }
        fin.close();
    }
    
    void saveToFile(string file_name) {
        
    }
    
    void _generateRequests() {
        requests.clear();
        for (int i = 0; i < N; ++i) {
            int l = i < L? i : rand() % L; // make sure every label has one buyer
            int d = rand() % D + 1;
            int v = rand() % MAX_VALUATION + 1;       // we assume v is [1, 100]
            requests.push_back(Request(l, d, v));
        }
    }
    
    void _generateUsers() {
        users.clear();
        for (int i = 0; i < M; ++i) {
            vector<int> user(0);
            assert(user.size() == 0);
            user.push_back(0); // make sure every user has label 0
            for (int l = 1; l < L - 2; ++l) {
                if (rand() % 10 < 3) {
                    user.push_back(l);
                }
            }
            if (user.back() == L - 3) {
                int c = rand() % 10;
                if (c < 3) {
                    user.push_back(L-1);
                } else if (c < 6) {
                    user.push_back(L-2);
                }
            }
            users.push_back(user);
        }
    }
};

#endif