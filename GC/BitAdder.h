/*
 * BitAdder.h
 *
 */

#ifndef GC_BITADDER_H_
#define GC_BITADDER_H_

#include "Processor/Processor.h"
#include "Processor/ThreadQueue.h"
#include <vector>
#ifdef RMFE_UNIT
#include "RmfeShare.h"
#endif
using namespace std;

class BitAdder
{
public:
    template<class T>
    void add(vector<vector<T>>& res, const vector<vector<vector<T>>>& summands,
            SubProcessor<T>& proc, int length, ThreadQueues* queues = 0,
            int player = -1);

    template<class T>
    void add(vector<vector<T>>& res, const vector<vector<vector<T>>>& summands,
            size_t begin, size_t end, SubProcessor<T>& proc, int length,
            int input_begin = -1, const void* supply = 0);

    template<class T>
    void multi_add(vector<vector<T>>& res, const vector<vector<vector<T>>>& summands,
            size_t begin, size_t end, SubProcessor<T>& proc, int length,
            int input_begin);

#ifdef RMFE_UNIT
    template<>
    void add(vector<vector<GC::RmfeShare>>& res, const vector<vector<vector<GC::RmfeShare>>>& summands,
            SubProcessor<GC::RmfeShare>& proc, int length, ThreadQueues* queues,
            int player);

    template<>
    void add(vector<vector<GC::RmfeShare>>& res, const vector<vector<vector<GC::RmfeShare>>>& summands,
            size_t begin, size_t end, SubProcessor<GC::RmfeShare>& proc, int length,
            int input_begin, const void* supply);
#endif

};

#endif /* GC_BITADDER_H_ */
