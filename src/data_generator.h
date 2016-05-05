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
const int MAX_VALUATION = 5;

struct NetworkData {
    int N; // number of buyers
    int M; // number of users
    int L; // number of labels
    int D; // number of max demand
    int L_user; // number of labels one user can have at most
    string prefix;
    vector<Request> requests;
    vector<User> users;
    
    NetworkData () {
    }
    
    void init(int N, int M, int L, int L_user) {
        srand(time(NULL));
        this->N = N;
        this->M = M;
        this->L = L;
        this->L_user = L_user;
        D = M * 4 / N;
        assert(N>=L);
        _generateRequests();
        _generateUsers();
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
    
    void saveToFile() {
        string file_name = "data/" + prefix + "_" + "0_revenue";
        ofstream fout(file_name);
        fout << M << " " << N << " " << L;
        for (const auto& request : requests) {
            int l = get<0>(request);
            int d = get<1>(request);
            int v = get<2>(request);
            fout << l << " " << d << " " << v << endl;
        }
        for (const auto& user : users) {
            fout << user.size();
            for (const auto& l : user) {
                fout << " " << l;
            }
            fout << endl;
        }
        fout.close();
    }
    
    void _generateRequests() {
        requests.clear();
        for (int i = 0; i < N; ++i) {
            int l = i < L? i : rand() % L; // make sure every label has one buyer
            int d = rand() % D + 1;
            //cout << d << endl;
            int v = rand() % MAX_VALUATION + 1;       // we assume v is [1, 100]
            requests.push_back(Request(l, d, v));
        }
    }
    
    void _generateUsers() {
        users.clear();
        vector<int> labels;
        
        // start from 1
        for (int l = 1; l < L; ++l) {
            labels.push_back(l);
        }
        for (int i = 0; i < M; ++i) {
            vector<int> user(0);
            assert(user.size() == 0);
            user.push_back(0); // make sure every user has label 0
            int n_l = rand() % L_user + 1;
            random_shuffle(labels.begin(), labels.end());
            // label 0 is already inserted
            for (int i = 0; i < n_l - 1; ++i) {
                user.push_back(labels[i]);
            }
            users.push_back(user);
        }
    }
};

#endif