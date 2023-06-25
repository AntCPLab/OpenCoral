
#ifndef PROCESSOR_ONLINEMACHINEEXT_H_
#define PROCESSOR_ONLINEMACHINEEXT_H_

#include "Processor/OnlineMachine.h"
#include "Processor/Machine.h"

class DishonestMajorityMachineExt : public DishonestMajorityMachine
{
public:
    template<class V>
    DishonestMajorityMachineExt(int argc, const char** argv,
            ez::ezOptionParser& opt, OnlineOptions& online_opts, V,
            int nplayers = 0): 
        DishonestMajorityMachine(argc, argv, opt, online_opts, V(), nplayers) 
        {
        }
    
    template<class sint, class sgf2n>
    Machine<sint, sgf2n> get_internal_machine() {
        return Machine<sint, sgf2n>(playerNames, use_encryption, online_opts, lg2);
    }
};

#endif /* PROCESSOR_ONLINEMACHINEEXT_H_ */
