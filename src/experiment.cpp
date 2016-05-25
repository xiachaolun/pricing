#include "data_generator.h"
#include "max_flow.h"
#include "min_cost_flow.h"
#include "approximate_min_cost_flow.h"
#include "solver.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <assert.h>
#include <string>
#include <map>
#include <time.h>
#include <cstring>

using namespace std;

void runEvaluation(int N, int M, int L, int L_user) {
    int n_cases = 1;
    while (true) {
        cout << "case: " << n_cases++ << endl;
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

void runExperiment2(int N, int M, int L, int L_user) {
    int count = 1000;
    while (count--) {
        NetworkData data;
        data.init(N,M,L,L_user);
        vector<int> p(0);
        for (int i = 0; i < L; ++i) {
            p.push_back(rand()%(MAX_VALUATION+1));
        }
        ProblemSolver ps(data);
        cout << ps._getRevenueForNonuniformPricing(p);
        ApproximateAlgorithm aa = ApproximateAlgorithm(data, p);
        cout << " " << aa.computeRevenue() << endl;
//        int random_max = 0;
//        for (int i = 0; i < 20; ++i) {
//            int r = aa._computeRevenueWithRandomSelection();
//            random_max = r > random_max ? r : random_max;
//            cout << " " << random_max;
//        }
//        cout << endl;
    }
}

void runExperiment22(int N, int M, int L, int L_user) {
    int count = 200;
    while (count--) {
        NetworkData data;
        data.init(N,M,L,L_user);
        vector<int> p(0);
        ProblemSolver ps(data);
        cout << ps.findOptimalUniformPrice().first << " ";
        pair<int, vector<int> > r1 = ps.findLocallyOptimalNonuiformPricing(0,0);
        pair<int, vector<int> > r2 = ps.findLocallyOptimalNonuiformPricing(1,0);
        cout << r1.first << " ";
        cout << ps._getRevenueForNonuniformPricing(r2.second) << endl;
    }
}

int main(int argc, char* argv[]) {
    srand(unsigned(time(0)));
    assert(argc == 5);
    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    int L = atoi(argv[3]);
    int L_user = atoi(argv[4]);
    cout << "Buyers: " << N << " Users: " << M << " L: " << L << " L per user: " << L_user << " Max Valution:" << MAX_VALUATION << endl;

    runEvaluation(N,M,L,L_user);
//    runExperiment22(N,M,L,L_user);
}
