#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <ctime>
#include <string>
#include <iostream>
#include <stack>
#include <utility>
#include <chrono>
#include <map>
#include <mutex>


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

inline size_t comm_log(const std::string& tag, size_t comm, bool print=true) {
    static std::stack<std::pair<std::string, size_t>> sentinel;
    
    if (sentinel.empty() || sentinel.top().first != tag) {
        auto start = comm;
        sentinel.push(make_pair(tag, start));
        return 0;
    }

    else {
        auto start = sentinel.top().second;
        auto end = comm;
        if (print) {
            std::cout << "[Comm] " << tag << ": " 
                << (end-start) * 1.0 / 1e6
                << " MB" << std::endl;
        }
        sentinel.pop();
        return end-start;
    }
}

inline std::pair<std::chrono::nanoseconds, size_t> perf_log(const std::string& tag, size_t comm, bool print=true) {
    auto t = time_log(tag, print);
    auto c = comm_log(tag, comm, print);
    return {t, c};
}

class ThreadPerformance {
public:
    std::string tag;
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::nanoseconds duration;
    size_t start_comm;
    size_t total_comm;

    ThreadPerformance(const std::string& tag, size_t start_comm = 0): tag(tag), start_comm(start_comm) {
        start(start_comm);
    }

    void start(size_t start_comm = 0) {
        start_time = std::chrono::high_resolution_clock::now();
    }

    void stop(size_t end_comm = 0) {
        auto end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start_time);
        total_comm = end_comm - start_comm;
    }
};

class GlobalPerformance {
public:
    static GlobalPerformance* singleton;

    std::map<std::string, std::chrono::nanoseconds> logs;
    std::map<std::string, size_t> comm_logs;
    std::mutex mtx;

    static GlobalPerformance& s() {
        if (singleton)
            return *singleton;
        else
            throw std::runtime_error("no singleton: GlobalPerformance");
    }

    GlobalPerformance() {
        if (singleton)
            throw std::runtime_error("there can only be one GlobalPerformance");
        singleton = this;
    }

    void add(const ThreadPerformance& log) {
        mtx.lock();
        logs[log.tag] += log.duration;
        comm_logs[log.tag] += log.total_comm;
        mtx.unlock();
    }

    void print_time(const std::string& tag="") {
        if (tag == "") {
            for(const auto& e : logs)
                print_time(e.first);
        }
        else {
            std::cout << "[Total Time] " << tag << ": " 
                << std::chrono::duration_cast<std::chrono::milliseconds>(logs[tag]).count() * 1.0
                << " ms" << std::endl;
        }
    }

    void print_comm(const std::string& tag="") {
        if (tag == "") {
            for(const auto& e : comm_logs)
                print_comm(e.first);
        }
        else {
            std::cout << "[Total Comm] " << tag << ": " 
                << comm_logs[tag] * 1.0 / 1e6
                << " MB" << std::endl;
        }
    }

};

#endif // PERFORMANCE_H
