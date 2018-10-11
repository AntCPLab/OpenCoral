// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

#include "FHEOffline/PairwiseMachine.h"
#include <valgrind/callgrind.h>

int main(int argc, const char** argv)
{
    CALLGRIND_STOP_INSTRUMENTATION;
    PairwiseMachine machine(argc, argv);
    CALLGRIND_START_INSTRUMENTATION;
    machine.run();
}
