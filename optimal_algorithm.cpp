#include "data_generator.h"
#include "check_acceptance.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <assert.h>

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
    printf("%.2lf, %.2lf\n", revenue, max_revenue);
    assert(revenue <= max_revenue + 1e-8);
    return 0;
}

int main() {
    auto segment_to_user = generateRandomAssigment();
    runGreedyAlgorithm(segment_to_user, generateRandomRequests(segment_to_user), generateRandomPrices());
}