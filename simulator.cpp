#include "data_generator.h"
#include "check_acceptance.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <assert.h>
#include <string>

double computeRevenue(const vector<pair<int, int> >& accepted_request_list,
                      const vector<double>& price_list) {
    double r = 0;
    for (const auto& request : accepted_request_list) {
        r += price_list[request.first] * request.second;
    }
    return r;
}

double runGreedyAlgorithm(const vector<vector<int> >& segment_to_user,
                          const vector<pair<int, int> >& request_list,
                          const vector<double>& price_list) {
    vector<pair<int, int> > current_request_list(0);
    vector<pair<double, pair<int, int> > > ordered_request_list(0);
    for (const auto& request : request_list) {
        double price = price_list[request.first];
        ordered_request_list.push_back(make_pair(price, request));
    }
    sort(ordered_request_list.begin(), ordered_request_list.end());
    reverse(ordered_request_list.begin(), ordered_request_list.end());
    
    double revenue = 0;
    double max_revenue = 0;
    
    for (const auto& p : ordered_request_list) {
        double price = p.first;
        pair<int, int> request = p.second;
        current_request_list.push_back(request);
        if (!isAcceptable(segment_to_user, current_request_list)) {
            current_request_list.pop_back();
        } else {
            revenue += price * request.second;
        }
        max_revenue += price * request.second;
        //printf("%.2lf, %d, %d\n", price, request.first, request.second);
    }
    //printf("%.2lf, %.2lf\n", revenue, max_revenue);
    assert(revenue <= max_revenue + 1e-8);
    return revenue;
}

vector<vector<pair<int, int> > > generateFeasibleSolutions(
    const vector<vector<int> >& segment_to_user,
    const vector<pair<int, int> >& request_list) {
    // enumerate all feasible solutions with at most 3 request lists
    vector<vector<pair<int, int> > > feasible_solutions(0);
    for (int i = 0; i< request_list.size(); i++) {
        vector<pair<int, int> > cql1(0); // current_request_list_1
        cql1.push_back(request_list[i]);
        if (isAcceptable(segment_to_user, cql1)) {
            feasible_solutions.push_back(cql1);
            for (int j = i + 1; j < request_list.size(); j++) {
                vector<pair<int, int> > cql2(cql1);
                cql2.push_back(request_list[j]);
                if (isAcceptable(segment_to_user, cql2)) {
                    feasible_solutions.push_back(cql2);
                    for (int k = j + 1; k < request_list.size(); k++) {
                        vector<pair<int, int> > cql3(cql2);
                        cql3.push_back(request_list[k]);
                        if (isAcceptable(segment_to_user, cql3)) {
                            feasible_solutions.push_back(cql3);
                        }
                    }
                }
            }
        }
    }
    return feasible_solutions;
}

double findBetterSolutionBasedOnFeasibleOne(const vector<vector<int> >& segment_to_user,
                                            const vector<pair<int, int> >& request_list,
                                            const vector<double>& price_list,
                                            const vector<pair<double, pair<int, int> > > ordered_request_pairs,
                                            const vector<pair<int, int> >& feasible_solution) {
    // feasible solution is a vector with at most 3 acceptable requests.
    vector<pair<int, int> > accepted_request_list(feasible_solution);
    for (const auto& request_pair : ordered_request_pairs){
        //double price = request_pair.first;
        const auto& request = request_pair.second;
        bool in_fs = 0;
        for (const auto fs : feasible_solution) {
            if (fs.first == request.first) { // this logic works if the data generator only generates requests with distinct segment ids.
                in_fs = 1;
                break;
            }
        }
        if (in_fs) {
            continue;
        }
        accepted_request_list.push_back(request);
        if (!isAcceptable(segment_to_user, accepted_request_list)) {
            accepted_request_list.pop_back();
        }
    }
    return computeRevenue(accepted_request_list, price_list);
}

pair<double, double> runGreedyAlgorithm2(const vector<vector<int> >& segment_to_user,
                           const vector<pair<int, int> >& request_list,
                           const vector<double>& price_list) {
    vector<pair<double, pair<int, int> > > ordered_request_pairs(0);
    for (const auto& request : request_list) {
        double price = price_list[request.first];
        ordered_request_pairs.push_back(make_pair(price, request));
    }
    sort(ordered_request_pairs.begin(), ordered_request_pairs.end());
    reverse(ordered_request_pairs.begin(), ordered_request_pairs.end());
    
    const auto& feasible_solutions = generateFeasibleSolutions(segment_to_user, request_list);
    double best_revenue_3 = -1, best_revenue_1 = -1;
    for (const auto& fs : feasible_solutions) {
        double revenue = findBetterSolutionBasedOnFeasibleOne(segment_to_user, request_list, price_list, ordered_request_pairs, fs);
        best_revenue_3 = revenue > best_revenue_3 ? revenue : best_revenue_3;
        if (fs.size() == 1) {
            best_revenue_1 = revenue > best_revenue_1 ? revenue : best_revenue_1;
        }
    }
    return make_pair(best_revenue_1, best_revenue_3);
}

double _G_best_revenue = 0;
vector<pair<int, int> > _G_current_request_list, _G_best_request_list;

void DFS(const vector<vector<int> >& segment_to_user,
         const vector<pair<int, int> >& request_list,
         const vector<double>& price_list,
         int l) {
    double current_revenue = computeRevenue(_G_current_request_list, price_list);
    if (current_revenue > _G_best_revenue) {
        _G_best_revenue = current_revenue;
        _G_best_request_list = _G_current_request_list;
    }
    if (l == request_list.size()) {
        return ;
    }
    
    DFS(segment_to_user, request_list, price_list, l+1);
    _G_current_request_list.push_back(request_list[l]);
    if (isAcceptable(segment_to_user, _G_current_request_list)) {
        DFS(segment_to_user, request_list, price_list, l+1);
    }
    _G_current_request_list.pop_back();
}

double runOptimalAlgorithm(const vector<vector<int> >& segment_to_user,
                           const vector<pair<int, int> >& request_list,
                           const vector<double>& price_list) {
    _G_best_revenue = 0;
    _G_current_request_list.clear();
    _G_best_request_list.clear();
    DFS(segment_to_user, request_list, price_list, 0);
    return _G_best_revenue;
}

int main() {

    int count = 0;
    double min_1 = 1, min_3 = 1;
    while (true) {
        const auto& segment_to_user = generateRandomAssigment();
        const auto& request_list = generateRandomRequests(segment_to_user);
        const auto& price_list = generateRandomPrices();
        double optimal = runOptimalAlgorithm(segment_to_user, request_list, price_list);
        if (optimal < 1e-10) {
            continue;
        }
        pair<double, double> greedy = runGreedyAlgorithm2(segment_to_user, request_list, price_list);
        double r_l_1 = greedy.first, r_l_3 = greedy.second;
        assert (r_l_1 <= optimal + 1e-10);
        assert (r_l_1 <= r_l_3 + 1e-10);
        assert (r_l_3 <= optimal + 1e-10);
        if (r_l_1 / optimal < min_1) {
            min_1 = r_l_1 / optimal;
        }
        if (r_l_3 / optimal < min_3) {
            min_3 = r_l_3 / optimal;
        }
        count++;
        if (count % 10 == 0) {
            cout << count << ": " << min_1 << " " << min_3 << endl;
        }
    }
    // 0.045, 0.366, 0.618
}