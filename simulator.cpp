#include "data_generator.h"
#include "check_acceptance.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <assert.h>

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

double runGreedyAlgorithm2(const vector<vector<int> >& segment_to_user,
                          const vector<pair<int, int> >& request_list,
                          const vector<double>& price_list) {
    double revenue = runGreedyAlgorithm(segment_to_user, request_list, price_list);
    double single_revenue = 0;
    vector<pair<int, int> > current_request_list(0);
    for (const auto& request : request_list) {
        current_request_list.push_back(request);
        if (isAcceptable(segment_to_user, current_request_list)) {
            single_revenue = price_list[request.first] * request.second;
            if (single_revenue > revenue) {
                revenue = single_revenue;
            }
        }
        current_request_list.pop_back();
    }
    
    return revenue;
}

double runGreedyAlgorithm3(const vector<vector<int> >& segment_to_user,
                          const vector<pair<int, int> >& request_list,
                          const vector<double>& price_list) {
    double revenue = runGreedyAlgorithm2(segment_to_user, request_list, price_list);
    
    double best_three_revenue = 0;
    for (int i = 0; i< request_list.size(); i++) {
        vector<pair<int, int> > current_request_list(0);
        current_request_list.push_back(request_list[i]);
        if (isAcceptable(segment_to_user, current_request_list)) {
            double three_revenue = computeRevenue(current_request_list, price_list);
            if (three_revenue > best_three_revenue) {
                best_three_revenue = three_revenue;
            }
        } else {
            continue;
        }
        for (int j = i + 1; j < request_list.size(); j++) {
            current_request_list.push_back(request_list[j]);
            if (isAcceptable(segment_to_user, current_request_list)) {
                double three_revenue = computeRevenue(current_request_list, price_list);
                if (three_revenue > best_three_revenue) {
                    best_three_revenue = three_revenue;
                }
            } else{
                current_request_list.pop_back();
                continue;
            }
            for (int k = j + 1; k < request_list.size(); k++){
                current_request_list.push_back(request_list[k]);
                if (isAcceptable(segment_to_user, current_request_list)) {
                    double three_revenue = computeRevenue(current_request_list, price_list);
                    if (three_revenue > best_three_revenue) {
                        best_three_revenue = three_revenue;
                    }
                }
                current_request_list.pop_back();
            }
            current_request_list.pop_back();
        }
    }
    
    if (best_three_revenue > revenue) {
        revenue = best_three_revenue;
    }

    return revenue;
}


double best_revenue = 0;
vector<pair<int, int> > current_request_list, best_request_list;

void DFS(const vector<vector<int> >& segment_to_user,
         const vector<pair<int, int> >& request_list,
         const vector<double>& price_list,
         int l) {
    double current_revenue = computeRevenue(current_request_list, price_list);
    if (current_revenue > best_revenue) {
        best_revenue = current_revenue;
        best_request_list = current_request_list;
    }
    if (l == request_list.size()) {
        return ;
    }
    
    DFS(segment_to_user, request_list, price_list, l+1);
    current_request_list.push_back(request_list[l]);
    if (isAcceptable(segment_to_user, current_request_list)) {
        DFS(segment_to_user, request_list, price_list, l+1);
    }
    current_request_list.pop_back();
}

double runOptimalAlgorithm(const vector<vector<int> >& segment_to_user,
                           const vector<pair<int, int> >& request_list,
                           const vector<double>& price_list) {
    best_revenue = 0;
    current_request_list.clear();
    best_request_list.clear();
    DFS(segment_to_user, request_list, price_list, 0);
    return best_revenue;
}

int main() {
    int count = 0;
    double min1=1, min2=1, min3=1;
    while (true) {
        const auto& segment_to_user = generateRandomAssigment();
        const auto& request_list = generateRandomRequests(segment_to_user);
        const auto& price_list = generateRandomPrices();
        double optimal = runOptimalAlgorithm(segment_to_user, request_list, price_list);
        double greedy = runGreedyAlgorithm(segment_to_user, request_list, price_list) / (optimal+1e-10);
        double greedy2 = runGreedyAlgorithm2(segment_to_user, request_list, price_list) / (optimal+1e-10);
        double greedy3 = runGreedyAlgorithm3(segment_to_user, request_list, price_list) / (optimal+1e-10);
        assert(greedy2>=greedy-1e-8);
        assert(greedy3>=greedy2-1e-8);
        assert(optimal>=greedy3-1e-8);
        min1 = greedy < min1 ? greedy : min1;
        min2 = greedy2 < min2 ? greedy2 : min2;
        min3 = greedy3 < min3 ? greedy3 : min3;
        count += 1;
        if (count % 50 == 0) {
            printf("%.3lf, %.3lf, %.3lf\n", min1, min2, min3);
        }
        //printf("%.2lf, %.2lf, %.2lf\n", greedy/(optimal+1e-10), greedy2/(optimal+1e-10), greedy3/(optimal+1e-10));
    }
}