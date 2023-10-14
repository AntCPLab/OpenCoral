#include "performance.h"

std::map<std::string, std::chrono::nanoseconds> profiling;
GlobalPerformance* GlobalPerformance::singleton = 0;