#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <ctime>
#include <string>
#include <iostream>
#include <stack>
#include <utility>
#include <chrono>
#include <map>


inline std::chrono::nanoseconds time_log(const std::string& tag, bool print=true) {
    static std::stack<std::pair<std::string, std::chrono::high_resolution_clock::time_point>> sentinel;
    
    if (sentinel.empty() || sentinel.top().first != tag) {
        auto start = std::chrono::high_resolution_clock::now();
        sentinel.push(make_pair(tag, start));
        return std::chrono::nanoseconds::zero();
    }

    else {
        auto start = sentinel.top().second;
        auto end = std::chrono::high_resolution_clock::now();
        if (print) {
            std::cout << "[Time] " << tag << ": " 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() * 1.0 
                << " ms" << std::endl;
        }
        sentinel.pop();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end-start);
    }
}

extern std::map<std::string, std::chrono::nanoseconds> profiling;
inline void acc_time_log(const std::string& tag) {
    profiling[tag] += time_log(tag, false);
}

inline void print_profiling(const std::string& tag="") {
    if (tag == "") {
        for(const auto& e : profiling)
            print_profiling(e.first);
    }
    else {
        std::cout << "[Total Time] " << tag << ": " 
            << std::chrono::duration_cast<std::chrono::milliseconds>(profiling[tag]).count() * 1.0
            << " ms" << std::endl;
    }
}

inline double get_acc_time_log(const std::string& tag) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(profiling[tag]).count() * 1.0;
}

#endif // PERFORMANCE_H
